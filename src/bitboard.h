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

#ifndef BITBOARD_H
#define BITBOARD_H

#include <cinttypes>

#include "magic_moves.h"
#include "types.h"

#define PRINT_BITBOARD(x) \
    for (int i = 0; i < 64; i++) { \
        printf("%d ", (((x) >> (i^56)) & 1) ? 1 : 0); \
        if (!((i + 1) % 8)) \
            putchar('\n'); \
    } \
    putchar('\n');

/* Bitboard masks for ranks on a chessboard. */
static const std::uint64_t rank_mask[8] = {
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
static const std::uint64_t file_mask[8] = {
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
extern std::uint64_t pawn_mask[2][64];
extern std::uint64_t knight_mask[64];
extern std::uint64_t king_mask[64];

#if defined(__GNUC__)
/* Get least significant bit. */
inline Square lsb(std::uint64_t bb)
{
    assert(bb);
    return Square(__builtin_ctzll(bb));
}

/* Get number of set bits. */
inline int popcnt(std::uint64_t bb)
{
    return __builtin_popcountll(bb);
}

/* Byte-swap a bitboard. */
inline std::uint64_t bswap(std::uint64_t bb)
{
    return __builtin_bswap64(bb);
}

#elif defined(_MSC_VER)

#include <intrin.h>
#include <stdlib.h> // Because MS put an intrinsic in the standard library...

/* Get least significant bit. */
inline Square lsb(std::uint64_t bb)
{
    assert(bb);
    unsigned long result = 64;

    _BitScanForward64(&result, bb);

#pragma warning(suppress: 4244)
    return Square(result);
}

/* Get number of set bits. */
inline int popcnt(std::uint64_t bb)
{
    return __popcnt64(bb);
}

/* Byte-swap a bitboard. */
inline std::uint64_t bswap(std::uint64_t bb)
{
    return _byteswap_uint64(bb);
}
#endif

/* Get attacks for a piece. */
template<Piece p>
std::uint64_t attacks(const Square sq, const std::uint64_t occ)
{
    static_assert(p != NO_PIECE, "attacks() called with NO_PIECE as parameter");

    switch (p) {
    case PAWN:
        return pawn_mask[US][sq];
    case KNIGHT:
        return knight_mask[sq];
    case BISHOP:
        return Bmagic(sq, occ);
    case ROOK:
        return Rmagic(sq, occ);
    case QUEEN:
        return Bmagic(sq, occ) | Rmagic(sq, occ);
    case KING:
        return king_mask[sq];
    case NO_PIECE:
        return 0;
    }
}

/* Get side-dependent pawn attacks. */
inline std::uint64_t pawn_attacks(const Square sq, const Colour c)
{
    return pawn_mask[c][sq];
}

extern void init_bitboards();

#endif

