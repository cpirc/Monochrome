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

#include "move.h"
#include "position.h"
#include "types.h"

void make_move(Position& pos, const Move move)
{
    static int const castling_lookup[64] = {
        13, 15, 15, 15, 12, 15, 15, 14,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        7,  15, 15, 15, 3,  15, 15, 11
    };

    pos.epsq = INVALID_SQUARE;

    const Square from = from_square(move),
                 to   = to_square(move);
    const MoveType mt = move_type(move);

    pos.castle &= castling_lookup[from] & castling_lookup[to];

    switch (mt) {
    case NORMAL:
        move_piece(pos, from, to, get_piece_on_square(pos, from), US);
        break;
    case CAPTURE:
        remove_piece(pos, to, get_piece_on_square(pos, to), THEM);
        move_piece(pos, from, to, get_piece_on_square(pos, from), US);
        break;
    case DOUBLE_PUSH:
        move_piece(pos, from, to, PAWN, US);
        pos.epsq = Square((unsigned char)(from) + 8);
        break;
    case ENPASSANT:
        move_piece(pos, from, to, PAWN, US);
        remove_piece(pos, Square((unsigned char)(to) - 8), PAWN, THEM);
        break;
    case CASTLE:
        move_piece(pos, from, to, KING, US);
        switch (to) {
        case C1:
            move_piece(pos, A1, D1, ROOK, US);
            break;
        case G1:
            move_piece(pos, H1, F1, ROOK, US);
            break;
        default:
            break;
        }
        break;
    case PROM_CAPTURE:
        remove_piece(pos, to, get_piece_on_square(pos, to), THEM);
        remove_piece(pos, from, PAWN, US);
        put_piece(pos, to, promotion_type(move), US);
        break;
    case PROMOTION:
        remove_piece(pos, from, PAWN, US);
        put_piece(pos, to, promotion_type(move), US);
        break;
    default:
        break;
    }
}
