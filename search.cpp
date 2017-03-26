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

#include "eval.h"
#include "move.h"
#include "position.h"

#define MAX_PLY (64)
#define INF     (30000)

/* A data structure to pass local parameters thru */
struct SearchStack {
    std::uint8_t ply;
    Move ml[256];
    int score[256];
};

int score_move(const Position& pos, Move m)
{
    /* MVV/LVA */
    Piece from = get_piece_on_square(pos, from_square(m));
    Piece dest = get_piece_on_square(pos, to_square(m));

    return piecevals[dest] - int(from);
}

/* Score a SearchStack. */
void score_moves(const Position& pos, SearchStack* ss, int size)
{
    for (int i = 0; i < size; i++) {
        ss->score[i] = score_move(pos, ss->ml[i]);
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

/* Alpha-Beta search a position to return a score. */
int search(Position& pos, int depth, int alpha, int beta, SearchStack* ss)
{
    if (ss->ply >= MAX_PLY) {
        return evaluate(pos);
    }

    Move* ml = ss->ml;
    int movecount, value;
    const bool quies = depth <= 0;
    const bool in_check = is_checked(pos, US);

    if (quies) {
        /* Stand pat. */
        value = evaluate(pos);

        if (value >= beta) {
            return beta;
        }
        if (value > alpha) {
            alpha = value;
        }

        movecount = generate_captures(pos, ml);
    } else {
        movecount = generate(pos, ml);
    }

    score_moves(pos, ss, movecount);

    int legal_moves = 0;
    Move best_move = 0;

    Move move;
    while ((move = next_move(ss, movecount))) {

        Position npos = pos;

        make_move(npos, move);
        if (is_checked(npos, THEM)) {
            continue;
        }

        ++legal_moves;

        value = -search(npos, depth - 1, -beta, -alpha, ss + 1);

        if (value >= beta) {
            return beta;
        }
        if (value > alpha) {
            alpha = value;
            best_move = move;
        }
    }

    if (!legal_moves) {
        if (in_check)
            return -INF + ss->ply;
        else
            return 0;
    }

    if (!ss->ply) {
        char mstr[6];
        move_to_lan(mstr, best_move);
        printf("best move: %s\n", mstr);
    }

    return alpha;
}

/* Reset the search stack to default values */
void clear_ss(SearchStack* ss, int size)
{
    for (std::uint8_t i = 0; i < size; ++i, ++ss) {
        ss->ply = i;
    }
}

/* Start searching a position and return the best move */
Move start_search(Position& pos)
{
    SearchStack ss[MAX_PLY];
    clear_ss(ss, MAX_PLY);
    for (int depth = 1; depth < MAX_PLY; ++depth) {
        int score = search(pos, depth, -INF, +INF, ss);
        printf("depth %d, score %d\n", depth, score);
    }
    Move move = 0;
    return move;
}
