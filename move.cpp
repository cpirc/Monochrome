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

#include "types.h"
#include "move.h"
#include "position.h"
#include "bitboard.h"

void make_move(Position& pos, const Move move)
{
    static unsigned char const castling_lookup[64] = {
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
        pos.epsq = from + 8;
        break;
    case ENPASSANT:
        move_piece(pos, from, to, PAWN, US);
        remove_piece(pos, to - 8, PAWN, THEM);
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

    flip_position(pos);
}


static const char *square_str[] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};

void move_to_lan(char* lan_str, const Move move)
{
    Square from = from_square(move),
             to = to_square(move);

    lan_str[0] = square_str[(std::size_t)from][0];
    lan_str[1] = square_str[(std::size_t)from][1];

    lan_str[2] = square_str[(std::size_t)to][0];
    lan_str[3] = square_str[(std::size_t)to][1];

    if (move_type(move) == PROMOTION) {
        switch (promotion_type(move)) {
        case KNIGHT:
            lan_str[4] = 'n';
            break;
        case BISHOP:
            lan_str[4] = 'b';
            break;
        case ROOK:
            lan_str[4] = 'r';
            break;
        case QUEEN:
            lan_str[4] = 'q';
            break;
        default:
            break;
        }
        lan_str[5] = '\0';
    } else
        lan_str[4] = '\0';
}

void run_move_to_lan_tests(void)
{
    char str[6];

    move_to_lan(str, get_move(H2, H1, PROMOTION, TO_ROOK));
    printf("%s\n", str);
    move_to_lan(str, get_move(A5, C7, NORMAL));
    printf("%s\n", str);
    move_to_lan(str, get_move(G7, G8, PROMOTION, TO_BISHOP));
    printf("%s\n", str);
    move_to_lan(str, get_move(E7, E8, PROMOTION, TO_QUEEN));
    printf("%s\n", str);
    move_to_lan(str, get_move(F2, F1, PROMOTION, TO_KNIGHT));
    printf("%s\n", str);
}

void lan_to_move(const char* lan_str, Move& move)
{
    Square from = (Square)( (lan_str[0] - 'a') + ( (lan_str[1] - '1') * 8 ) ),
             to = (Square)( (lan_str[2] - 'a') + ( (lan_str[3] - '1') * 8 ) );

    if (from < INVALID_SQUARE && to < INVALID_SQUARE) {
        switch (lan_str[4]) {
        case 'n':
            move = get_move(from, to, PROMOTION, TO_KNIGHT);
            break;
        case 'b':
            move = get_move(from, to, PROMOTION, TO_BISHOP);
            break;
        case 'r':
            move = get_move(from, to, PROMOTION, TO_ROOK);
            break;
        case 'q':
            move = get_move(from, to, PROMOTION, TO_QUEEN);
            break;
        default:
            move = get_move(from, to, NORMAL);
            break;
        }
    }
}
