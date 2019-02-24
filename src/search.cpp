/*
MIT License

Copyright (c) 2017 CPirc
Copyright (c) 2018 CPirc
Copyright (c) 2019 CPirc

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
#include <algorithm> // std::reverse

#include "eval.h"
#include "move.h"
#include "position.h"
#include "search.h"

/* MVV/LVA */
int mvv_lva(const Position& pos, Move m)
{
    Piece from = get_piece_on_square(pos, from_square(m));
    Piece dest = get_piece_on_square(pos, to_square(m));

    return piecevals[OPENING][dest] - from;
}

/* Score a SearchStack. */
void score_moves(const Position& pos, SearchStack* ss, int size, Move hash_move)
{
    for (int i = 0; i < size; i++) {
        Move move = ss->ml[i];

        if (move == hash_move) {
            ss->score[i] = 20000;
            continue;
        }

        int mt = move_type(move);
        if (mt == CAPTURE)
            ss->score[i] = 9000 + mvv_lva(pos, move);
        else if (mt == PROM_CAPTURE)
            ss->score[i] = 9000 + mvv_lva(pos, move) + piecevals[OPENING][promotion_type(move)];
        else if (mt == ENPASSANT)
            ss->score[i] = 9000 + piecevals[OPENING][PAWN] - PAWN + 10;
	else if (ss->killers[0] == move)
            ss->score[i] = 7000;
	else if (ss->killers[1] == move)
            ss->score[i] = 6000;
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
int quiesce(SearchController& sc, Position& pos, int alpha, int beta, SearchStack* ss)
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

    score_moves(pos, ss, movecount, 0);

    Move move;
    while ((move = next_move(ss, movecount))) {

        Position npos = pos;

        make_move(npos, move);
        if (is_checked(npos, THEM)) {
            continue;
        }

        value = -quiesce(sc, npos, -beta, -alpha, ss + 1);

        if (value >= beta) {
            return beta;
        }
        if (value > alpha) {
            alpha = value;
        }
    }

    return alpha;
}

/* Alpha-Beta search a position to return a score. */
template<bool pv_node = true>
int search(SearchController& sc, Position& pos, int depth, int alpha, int beta, SearchStack* ss, PV& pv)
{
    if (is_threefold(pos) || is_fifty_moves(pos)) {
        return 0;
    }

    const bool in_check = is_checked(pos, US);

    // Check extensions
    if (in_check)
        depth++;

    if (depth <= 0) {
        return quiesce(sc, pos, alpha, beta, ss);
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

    ++ss->stats->node_count;

    // Check transposition table
    Move hash_move = 0;
    TTEntry entry = tt_poll(&sc.tt, pos.hash_key);

    if (entry.hash_key == pos.hash_key) {
        int entry_depth = tt_depth(entry.data);
        hash_move = tt_move(entry.data);

        if (!pv_node && entry_depth >= depth) {
            int entry_eval = eval_from_tt(tt_eval(entry.data), ss->ply);
            int entry_flag = tt_flag(entry.data);

            if ( entry_flag == TT_EXACT ||
                (entry_flag == TT_LOWER && entry_eval >= beta) ||
                (entry_flag == TT_UPPER && entry_eval <= alpha)) {

                pv.push_back(hash_move);
                return entry_eval;
            }
        }
    }

    int movecount, value;
    movecount = generate(pos, ss->ml);

    score_moves(pos, ss, movecount, hash_move);

    int legal_moves = 0;
    int best_value = -INF;
    int old_alpha = alpha;

    Move move;
    Move best_move = 0;
    PV child_pv;
    while ((move = next_move(ss, movecount))) {

        child_pv.clear();
        Position npos = pos;

        make_move(npos, move);
        if (is_checked(npos, THEM)) {
            continue;
        }

        ++legal_moves;

        if (legal_moves == 1)
            value = -search(sc, npos, depth - 1, -beta, -alpha, ss + 1, child_pv);
        else
            value = -search<false>(sc, npos, depth - 1, -beta, -alpha, ss + 1, child_pv);

        if (value > best_value) {
            best_value = value;
            best_move = move;
            child_pv.push_back(move);
            pv = std::move(child_pv);

            if (value > alpha) {
                alpha = value;
            }
        }
        if (value >= beta) {
#ifdef TESTING
            ++ss->stats->fail_highs;
            if (legal_moves == 1)
                ++ss->stats->first_move_fail_highs;
#endif
            if (  !promotion_type(move)
                && move_type(move) != CAPTURE
                && move_type(move) != ENPASSANT
		&& ss->killers[0] != move) {
                ss->killers[1] = ss->killers[0];
                ss->killers[0] = move;
	    }
            tt_add(&sc.tt, pos.hash_key, move, depth, TT_LOWER, eval_to_tt(value, ss->ply));
            return beta;
        }
    }

    if (!legal_moves) {
        if (in_check)
            return -INF + ss->ply;
        else
            return 0;
    }

    // Add entry to transposition table
    int flag = alpha == old_alpha ? TT_UPPER : TT_EXACT;
    tt_add(&sc.tt, pos.hash_key, best_move, depth, flag, eval_to_tt(best_value, ss->ply));

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
        ss->killers[0] = ss->killers[1] = 0;
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

#define GUESSED_LENGTH 40

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
        sc.search_end_time = (sc.increment * (sc.moves_per_session - 1) + sc.our_clock) / sc.moves_per_session;
    } else {
        sc.search_end_time = (sc.increment * (GUESSED_LENGTH - 1) + sc.our_clock) / GUESSED_LENGTH;
    }

    sc.search_end_time += sc.search_start_time;

    char mstr[6];
    Move best_move;
    int best_score = -INF;
    PV pv, depth_pv, child_pv;

    /* Iterative deepening */
    for (std::uint32_t depth = 1; depth < sc.max_depth; ++depth) {

        int beta = INF;
        int alpha = -INF;

        int depth_best_score = search(sc, sc.pos, depth, alpha, beta, ss, depth_pv);

        // Check time used
        clock_t time_used = clock() * 1000 / CLOCKS_PER_SEC  - sc.search_start_time;

        // See if we ran out of time
        if (depth > 1 && time_used >= sc.search_end_time - sc.search_start_time) {
            break;
        }

        // Only update the best pv if we didn't run out of time
        // It needs reversing due to the last in first out nature of push_back()
        pv = depth_pv;
        std::reverse(pv.begin(), pv.end());

        best_score = depth_best_score;
        
        bool mate = false;
        if (best_score > INF - MAX_PLY) {
            mate = true;
            best_score = INF - best_score;
        }
        if (best_score < -INF + MAX_PLY) {
            mate = true;
            best_score = -INF + best_score;
        }

        // Update info
        if (mate) {
            printf("info score mate %i depth %i nodes %" PRIu64 " time %lu pv ", best_score, depth, stats.node_count, time_used);
        } else {
            printf("info score cp %i depth %i nodes %" PRIu64 " time %lu pv ", best_score, depth, stats.node_count, time_used);
        }
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

        // Exit if mate found
        if (mate) {
            break;
        }
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
