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

#ifndef TYPES_H
#define TYPES_H

#include <cassert>
#include <cinttypes>

/* Pieces on a chessboard. */
enum Piece : unsigned char {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NO_PIECE
};

/* Colours on a chessboard: */
enum Colour : unsigned char {
    US, THEM
};

/* Squares on a chessboard */
enum Square : unsigned char {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    INVALID_SQUARE
};

/* Ranks on a chessboard. */
enum Rank : unsigned char {
    RANK_1, RANK_2, RANK_3, RANK_4,
    RANK_5, RANK_6, RANK_7, RANK_8
};

/* Files on a chessboard. */
enum File : unsigned char {
    FILE_A, FILE_B, FILE_C, FILE_D,
    FILE_E, FILE_F, FILE_G, FILE_H
};

<<<<<<< HEAD
/* Castling rights. */
enum {
    US_OO    = 1 << 3,
    US_OOO   = 1 << 2,
    THEM_OO  = 1 << 1,
    THEM_OOO = 1 << 0
};

#define ENABLE_OPERATIONS(T)                                                   \
constexpr inline T operator+(T l, T r)   { return T((int)(l) + (int)(r)); }    \
constexpr inline T operator+(T l, int r) { return T((int)(l) + r);        }    \
constexpr inline T operator+(int l, T r) { return T(l + (int)(r));        }    \
constexpr inline T operator-(T l, T r)   { return T((int)(l) - (int)(r)); }    \
constexpr inline T operator-(T l, int r) { return T((int)(l) - r);        }    \
constexpr inline T operator-(int l, T r) { return T(l - (int)(r));        }    \
inline T operator+=(T& l, T r)           { return l = l + r;              }    \
inline T operator-=(T& l, T r)           { return l = l - r;              }    \
inline T operator++(T& l)                { return l = l + 1;              }    \
inline T operator--(T& l)                { return l = l - 1;              }

ENABLE_OPERATIONS(Piece)
ENABLE_OPERATIONS(Colour)
ENABLE_OPERATIONS(Square)
ENABLE_OPERATIONS(Rank)
ENABLE_OPERATIONS(File)

#endif
