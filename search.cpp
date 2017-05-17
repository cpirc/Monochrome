/*
MIT License

Copyright (c) 2017 CPirc

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <chrono>
#include <cinttypes>
#include <vector>

#include "eval.h"
#include "move.h"
#include "position.h"
#include "search.h"

#define INF     (30000)

/* A principal variation. */
typedef std::vector<Move> PV;

/* MVV/LVA */
int mvv_lva(const Position& pos, Move m)
{
    Piece from = get_piece_on_square(pos, from_square(m));
    Piece dest = get_piece_on_square(pos, to_square(m));

    return piecevals[OPENING][dest] - from;
}

/* Score a SearchStack. */
void score_moves(const Position& pos, SearchStack* ss, int size)
{
    for (int i = 0; i < size; i++) {
        Move move = ss->ml[i];
        int mt = move_type(move);
        if (mt == CAPTURE)
            ss->score[i] = mvv_lva(pos, move);
        else if (mt == PROM_CAPTURE)
            ss->score[i] = mvv_lva(pos, move) + piecevals[OPENING][promotion_type(move)];
        else if (mt == ENPASSANT)
            ss->score[i] = piecevals[OPENING][PAWN] - PAWN + 10;
        else
            ss->score[i] = 0;
    }
}

/* Return the best move from the search stack */
Move next_move(SearchStack* ss, int& size)
{
    if (!size)
        return 0;

    // Find best move index
    int best_index = 0;
    for (int i = 1; i < size; ++i) {
        if (ss->score[i] > ss->score[best_index]) {
            best_index = i;
        }
    }

    // Store the best move to return
    Move best_move = ss->ml[best_index];

    // Reduce ml/score size
    --size;

    // Bring the last move to current index
    ss->ml[best_index] = ss->ml[size];
    ss->score[best_index] = ss->score[size];

    return best_move;
}

/* Quiescence alpha-beta search a search leaf node to reduce the horizon effect. */
int quiesce(SearchController& sc, Position& pos, int alpha, int beta, SearchStack* ss, PV& pv)
{
    if (ss->ply >= MAX_PLY) {
        return evaluate(pos);
    }

    // Check time left
    clock_t current_time = clock() * 1000 / CLOCKS_PER_SEC;
    if (current_time >= sc.search_end_time) {
        return 0;
    }

    int movecount, value;

    value = evaluate(pos);
    if (value >= beta)
        return beta;

    if (value > alpha)
        alpha = value;

    movecount = generate_captures(pos, ss->ml);

    ++ss->stats->node_count;

    score_moves(pos, ss, movecount);

    Move move;
    PV child_pv;
    while ((move = next_move(ss, movecount))) {

        child_pv.clear();
        Position npos = pos;

        make_move(npos, move);
        if (is_checked(npos, THEM)) {
            continue;
        }

        value = -quiesce(sc, npos, -beta, -alpha, ss + 1, child_pv);

        if (value >= beta) {
            return beta;
        }
        if (value > alpha) {
            alpha = value;
            child_pv.push_back(move);
            pv = std::move(child_pv);
        }
    }

    return alpha;
}

/* Alpha-Beta search a position to return a score. */
int search(SearchController& sc, Position& pos, int depth, int alpha, int beta, SearchStack* ss, PV& pv)
{
    if (depth <= 0) {
        return quiesce(sc, pos, alpha, beta, ss, pv);
    }

    if (ss->ply >= MAX_PLY) {
        return evaluate(pos);
    }

    // Check time left
    clock_t current_time = clock() * 1000 / CLOCKS_PER_SEC;
    if (ss->ply && current_time >= sc.search_end_time) {
        return 0;
    }

    // Update info
    if (ss->stats->node_count%1048576 == 0) {
        if (current_time > sc.search_start_time) {
            printf("info nps %" PRIu64 "\n", 1000*(ss->stats->node_count)/(current_time - sc.search_start_time));
        }
    }

    int movecount, value;
    const bool in_check = is_checked(pos, US);
    movecount = generate(pos, ss->ml);

    ++ss->stats->node_count;

    score_moves(pos, ss, movecount);

    int legal_moves = 0;
    Move best_move = 0; // reserved for future use.

    Move move;
    PV child_pv;
    while ((move = next_move(ss, movecount))) {

        child_pv.clear();
        Position npos = pos;

        make_move(npos, move);
        if (is_checked(npos, THEM)) {
            continue;
        }

        ++legal_moves;

        value = -search(sc, npos, depth - 1, -beta, -alpha, ss + 1, child_pv);

        if (value >= beta) {
#ifdef TESTING
            ++ss->stats->fail_highs;
            if (legal_moves == 1)
                ++ss->stats->first_move_fail_highs;
#endif
            return beta;
        }
        if (value > alpha) {
            alpha = value;
            best_move = move;
            child_pv.push_back(move);
            pv = std::move(child_pv);
        }
    }

    if (!legal_moves) {
        if (in_check)
            return -INF + ss->ply;
        else
            return 0;
    }

#ifdef TESTING
    if (!ss->ply) {
        printf("info string ordering = %lf\n", double(ss->stats->first_move_fail_highs) / ss->stats->fail_highs);
    }
#endif

    return alpha;
}

/* Reset the stats object to 0 so we can start recording */
void clear_stats(Stats& stats)
{
    stats.node_count = 0;
#ifdef TESTING
    stats.fail_highs = 0;
    stats.first_move_fail_highs = 0;
#endif
}

/* Reset the search stack to default values */
void clear_ss(SearchStack* ss, int size)
{
    for (std::uint8_t i = 0; i < size; ++i, ++ss) {
        ss->ply = i;
    }
}

/* Set the Stats pointer for all ply after 'stats' */
void set_stats(SearchStack* ss, Stats& stats)
{
    SearchStack* end = ss - ss->ply + MAX_PLY;
    for (; ss < end; ++ss) {
        ss->stats = &stats;
    }
}

/* Start searching a position and return the best move */
Move start_search(SearchController& sc)
{
    Stats stats;
    SearchStack ss[MAX_PLY];
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;

    clear_stats(stats);
    clear_ss(ss, MAX_PLY);

    set_stats(ss, stats);

    /* Timing */
    sc.search_start_time = 1000 * clock() / CLOCKS_PER_SEC;

    if (sc.movetime) {
        sc.search_end_time = sc.movetime/2;
    } else if (sc.moves_per_session) {
        sc.search_end_time = sc.our_clock/sc.moves_per_session + sc.increment;
    } else {
        sc.search_end_time = sc.our_clock/40 + sc.increment;
    }

    sc.search_end_time += sc.search_start_time;

    char mstr[6];
    Move best_move;
    int best_score = -INF;
    PV pv, child_pv;

    /* Iterative deepening */
    for (std::uint32_t depth = 1; depth < sc.max_depth; ++depth) {

        int beta = INF;
        int alpha = -INF;
        int depth_best_score = -INF;
        Move depth_best_move = 0;

        /* Unroll first depth */
        int movecount = generate(sc.pos, ss->ml);
        score_moves(sc.pos, ss, movecount);

        Move move;
        while ((move = next_move(ss, movecount))) {

            child_pv.clear();
            Position npos = sc.pos;

            make_move(npos, move);
            if (is_checked(npos, THEM)) {
                continue;
            }

            int score = -search(sc, npos, depth - 1, -beta, -alpha, ss + 1, child_pv);

            if (score >= depth_best_score) {
                depth_best_score = score;
                depth_best_move = move;
                child_pv.push_back(move);
                pv = std::move(child_pv);
            }
        }

        // Check time used
        clock_t time_used = clock() * 1000 / CLOCKS_PER_SEC  - sc.search_start_time;

        // See if we ran out of time
        if (depth > 1 && time_used >= sc.search_end_time - sc.search_start_time) {
            break;
        }

        best_score = depth_best_score;

        // Update info
        printf("info score cp %i depth %i nodes %" PRIu64 " time %lu pv ", best_score, depth, stats.node_count, time_used);
        bool flipped = sc.pos.flipped;
        for (Move move : pv) {
            if (flipped) {
                move = flip_move(move);
            }
            move_to_lan(mstr, move);
            printf("%s ", mstr);
            flipped ^= 1;
        }
	printf("\n");
    }

    if (pv.size() >= 1) {
        best_move = pv[0];
        if (sc.pos.flipped) {
            best_move = flip_move(best_move);
        }
        move_to_lan(mstr, best_move);

        printf("bestmove %s\n", mstr);
    } else {
        printf("bestmove 0000\n");
    }

    return (Move)0;
}

void search_thread(void* params)
{
    SearchController *sc = (SearchController*)params;
    start_search(*sc);
}
