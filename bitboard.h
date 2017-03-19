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

#ifndef BITBOARD_H
#define BITBOARD_H

#include <cinttypes>

#include "magic_moves.h"
#include "types.h"

/* Bitboard masks for ranks on a chessboard. */
static const std::uint64_t RankMask[8] = {
    0x00000000000000FFULL,
    0x000000000000FF00ULL,
    0x0000000000FF0000ULL,
    0x00000000FF000000ULL,
    0x000000FF00000000ULL,
    0x0000FF0000000000ULL,
    0x00FF000000000000ULL,
    0xFF00000000000000ULL
};

/* Bitboard masks for files on a chessboard. */
static const std::uint64_t FileMask[8] = {
    0x0101010101010101ULL,
    0x0202020202020202ULL,
    0x0404040404040404ULL,
    0x0808080808080808ULL,
    0x1010101010101010ULL,
    0x2020202020202020ULL,
    0x4040404040404040ULL,
    0x8080808080808080ULL
};

/* Precalculated piece attacks for a square. */
extern std::uint64_t PawnMask[64];
extern std::uint64_t KnightMask[64];
extern std::uint64_t KingMask[64];

/* Get attacks for a piece. */
template<Piece p>
std::uint64_t Attacks(const Square sq, const std::uint64_t occ);

/* Get pawn attacks. */
template<>
inline std::uint64_t Attacks<PAWN>(const Square sq, const std::uint64_t)
{
    return PawnMask[sq];
}

/* Get knight attacks. */
template<>
inline std::uint64_t Attacks<KNIGHT>(const Square sq, const std::uint64_t)
{
    return KnightMask[sq];
}

/* Get bishop attacks. */
template<>
inline std::uint64_t Attacks<BISHOP>(const Square sq, const std::uint64_t occ)
{
    return Bmagic(sq, occ);
}

/* Get rook attacks. */
template<>
inline std::uint64_t Attacks<ROOK>(const Square sq, const std::uint64_t occ)
{
    return Rmagic(sq, occ);
}

/* Get queen attacks. */
template<>
inline std::uint64_t Attacks<QUEEN>(const Square sq, const std::uint64_t occ)
{
    return Bmagic(sq, occ) | Rmagic(sq, occ);
}

/* Get king attacks. */
template<>
inline std::uint64_t Attacks<KING>(const Square sq, const std::uint64_t)
{
    return KingMask[sq];
}

#endif

