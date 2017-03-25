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

/* Alpha-Beta search a position to return a score. */
int search(Position& pos, int depth, int alpha, int beta)
{
    Position npos;
    Move ml[256];
    int movecount, i, value;

    if (depth == 0) {
        return evaluate(pos);
    }

    movecount = generate(pos, ml);

    for (i = 0; i < movecount; i++) {

        npos = pos;

        make_move(npos, ml[i]);
        if (is_checked(npos, THEM)) {
            continue;
        }

        value = -search(npos, depth - 1, -beta, -alpha);

        if (value >= beta) {
            return beta;
        }
        if (value > alpha) {
            alpha = value;
        }
    }

    return alpha;
}
