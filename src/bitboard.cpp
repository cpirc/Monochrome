/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Dan Ravensloft <dan.ravensloft@gmail.com>
 * Copyright (c) 2017 CPirc
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

#include <cassert>
#include <cinttypes>

#include "bitboard.h"
#include "types.h"

std::uint64_t pawn_mask[2][64];
std::uint64_t knight_mask[64];
std::uint64_t king_mask[64];

void init_bitboards()
{
    initmagicmoves();

    // Pawns
    for (int sq = A1; sq <= H8; sq++) {
        std::uint64_t from = 1ULL << sq;
        pawn_mask[US][sq] = ((from << 7) & ~file_mask[FILE_H]) | ((from << 9) & ~file_mask[FILE_A]);
        pawn_mask[THEM][sq] = ((from >> 7) & ~file_mask[FILE_A]) | ((from >> 9) & ~file_mask[FILE_H]);
    }

    // Knights
    for (int sq = A1; sq <= H8; sq++) {
        std::uint64_t from = 1ULL << sq;
        knight_mask[sq]  = (from>>17) & (~file_mask[FILE_H]); // Up 2 right 1
        knight_mask[sq] |= (from>>15) & (~file_mask[FILE_A]); // Up 2 left 1
        knight_mask[sq] |= (from<<17) & (~file_mask[FILE_A]); // Down 2 left 1
        knight_mask[sq] |= (from<<15) & (~file_mask[FILE_H]); // Down 2 right 1
        knight_mask[sq] |= (from>>10) & ~(file_mask[FILE_G]|file_mask[FILE_H]); // Right 2 up 1
        knight_mask[sq] |= (from<<6)  & ~(file_mask[FILE_G]|file_mask[FILE_H]); // Right 2 down 1
        knight_mask[sq] |= (from>>6)  & ~(file_mask[FILE_A]|file_mask[FILE_B]); // Left 2 up 1
        knight_mask[sq] |= (from<<10) & ~(file_mask[FILE_A]|file_mask[FILE_B]); // Left 2 up 1
        knight_mask[sq] |= (from<<10) & ~(file_mask[FILE_A]|file_mask[FILE_B]); // Left 2 down 1
    }

    // Kings
    for (int sq = A1; sq <= H8; sq++) {
        std::uint64_t from = 1ULL << sq;
        king_mask[sq]  = (from>>8); // Up 1
        king_mask[sq] |= (from<<8); // Down 1
        king_mask[sq] |= (from>>1) & (~file_mask[FILE_H]); // Right 1
        king_mask[sq] |= (from<<1) & (~file_mask[FILE_A]); // Left 1
        king_mask[sq] |= (from>>7) & (~file_mask[FILE_A]); // Up 1 Left 1
        king_mask[sq] |= (from>>9) & (~file_mask[FILE_H]); // Up 1 Right 1
        king_mask[sq] |= (from<<7) & (~file_mask[FILE_H]); // Down 1 Right 1
        king_mask[sq] |= (from<<9) & (~file_mask[FILE_A]); // Down 1 Left 1
    }
}

