/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Dan Ravensloft <dan.ravensloft@gmail.com>
 * Copyright (c) 2018 CPirc
 * Copyright (c) 2019 CPirc
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <cinttypes>

#include "position.h"
#include "bitboard.h"
#include "move.h"

std::uint64_t perft(const Position& pos, const int depth)
{
    if (depth == 0) {
        return 1;
    }

    uint64_t nodes = 0;
    Move moves[256];
    int movecount = generate(pos, moves);

    for (int i = 0; i < movecount; i++) {
        Position npos = pos;

        make_move(npos, moves[i]);
        if (is_checked(npos, THEM))
            continue;

        nodes += perft(npos, depth - 1);
    }

    return nodes;
}

std::uint64_t perft_tt(TT* tt, const Position& pos, const int depth)
{
    if (depth == 0) {
        return 1;
    }

    TTEntry entry = tt_poll(tt, pos.hash_key);
    if (entry.hash_key == pos.hash_key && tt_depth(entry.data) == depth) {
        return tt_nodes(entry.data);
    }

    uint64_t nodes = 0;
    Move moves[256];
    int movecount = generate(pos, moves);

    for (int i = 0; i < movecount; i++) {
        Position npos = pos;

        make_move(npos, moves[i]);
        if (is_checked(npos, THEM))
            continue;

        nodes += perft_tt(tt, npos, depth - 1);
    }

    tt_add_perft(tt, pos.hash_key, depth, nodes);

    return nodes;
}

void run_perft_tests()
{
    Position pos;

    parse_fen_to_position((const char*)"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", pos);

    for (int i = 1; i <= 6; i++) {
        std::printf("Perft(%d) = %" PRIu64 "\n", i, perft(pos, i));
    }
}
