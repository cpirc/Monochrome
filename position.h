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

#ifndef POSITION_H
#define POSITION_H

#include "types.h"

/* A chess position. */
struct Position {
    std::uint64_t pieces[6];  // Bitboards containing piece locations.
    std::uint64_t colours[2]; // Bitboards containing colours of pieces.
    bool flipped;             // Side to move.
    std::uint8_t castle;      // Castling rights.
    Square epsq;              // En passant square.
    char fifty;               // Fifty-move rule counter.
    std::uint64_t hash_key;   // Zobrist hash of the current position.
};

/* Get a piece bitboard. */
static inline std::uint64_t get_piece(const Position& b, const Piece p)
{
    return b.pieces[int(p)];
}

/* Get a colour bitboard. */
static inline std::uint64_t get_colour(const Position& b, const Colour c)
{
    return b.colours[int(c)];
}

/* Get a piece bitboard of a colour. */
static inline std::uint64_t get_piece(const Position& b, const Piece p, const Colour c)
{
    return get_piece(b, p) & get_colour(b, c);
}

/* Set a bit in a piece bitboard. */
static inline void set_bit(Position& b, const Piece p, const Square sq)
{
    b.pieces[int(p)] |= 1ULL << sq;
}

/* Set a bit in a colour bitboard. */
static inline void set_bit(Position& b, const Colour c, const Square sq)
{
    b.colours[int(c)] |= 1ULL << sq;
}

/* Clear a bit in a piece bitboard. */
static inline void clear_bit(Position& b, const Piece p, const Square sq)
{
    b.pieces[int(p)] &= ~(1ULL << sq);
}

/* Clear a bit in a colour bitboard. */
static inline void clear_bit(Position& b, const Colour c, const Square sq)
{
    b.colours[int(c)] &= ~(1ULL << sq);
}

#endif
