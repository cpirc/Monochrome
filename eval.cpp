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

#include <cinttypes>

#include "eval.h"
#include "position.h"
#include "types.h"

/* Piece values for material in centipawns. */
const int piecevals[7] = {
    100, 300, 300, 500, 900, 0, 0
};

/* Return material balance of a board. */
/* TODO: incremental update this. */
template<Piece p = PAWN>
int evaluate_material(const Position& pos)
{
    std::uint64_t pieces = get_piece(pos, p, US);

    return (popcnt(pieces) * piecevals[p]) + (p == QUEEN ? 0 : evaluate_material<p+1>(pos));
}

/* Return the heuristic value of a position. */
int evaluate(Position& pos)
{
    int score = 0;
    Colour side;

    for (side = US; side <= THEM; ++side) {
        score += evaluate_material<>(pos);

        score = -score;
        flip_position(pos);
    }

    return pos.flipped ? -score : score;
}

