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

#include "bitboard.h"
#include "move.h"
#include "position.h"
#include "types.h"

/* Castling occupancy masks. */
static const std::uint64_t oo_castle_mask = (1ULL << F1) | (1ULL << G1);
static const std::uint64_t ooo_castle_mask = (1ULL << D1) | (1ULL << C1) | (1ULL << B1);

/* Generic move serialisation loop. */
template<bool captures, Piece pc = PAWN>
void add_moves(const Position & pos, Move* ml, int& idx)
{
    std::uint64_t pieces = get_piece(pos, pc, US);
    std::uint64_t occ = get_occupancy(pos);
    std::uint64_t capturemask = (captures) ? get_colour(pos, THEM) : ~occ;
    MoveType mt = captures ? CAPTURE : NORMAL;

    while (pieces) {
        Square from = lsb(pieces);

        std::uint64_t attack_bb = attacks<pc>(from, occ) & capturemask;

        while (attack_bb) {
            Square dest = lsb(attack_bb);

            ml[idx] = get_move(from, dest, mt);
            idx++;

            attack_bb &= attack_bb - 1;
        }

        pieces &= pieces - 1;
    }

    add_moves<captures, pc+1>(pos, ml, idx);
}

/* Specialisation for pawn quiets. */
/* (promotions, pawn pushing) */
template<>
void add_moves<false, PAWN>(const Position& pos, Move* ml, int& idx)
{
    std::uint64_t pawns = get_piece(pos, PAWN, US);
    std::uint64_t empty = ~get_occupancy(pos);
    std::uint64_t singles, doubles;

    // Single push
    singles = (pawns << 8) & empty;

    // Separate promotions
    singles &= ~rank_mask[RANK_8];

    while (singles) {
        Square dest = lsb(singles);

        ml[idx] = get_move(dest - 8, dest, NORMAL);
        idx++;

        singles &= singles - 1;
    }

    // Double push
    singles = ((pawns & rank_mask[RANK_2]) << 8) & empty;
    doubles = (singles << 8) & empty;

    while (doubles) {
        Square dest = lsb(doubles);

        ml[idx] = get_move(dest - 16, dest, DOUBLE_PUSH);
        idx++;

        doubles &= doubles - 1;
    }

    // Promotions
    singles = ((pawns & rank_mask[RANK_7]) << 8) & empty;

    while (singles) {
        Square dest = lsb(singles);

        ml[idx] = get_move(dest - 8, dest, PROMOTION, TO_KNIGHT);
        idx++;

        ml[idx] = get_move(dest - 8, dest, PROMOTION, TO_BISHOP);
        idx++;

        ml[idx] = get_move(dest - 8, dest, PROMOTION, TO_ROOK);
        idx++;

        ml[idx] = get_move(dest - 8, dest, PROMOTION, TO_QUEEN);
        idx++;

        singles &= singles - 1;
    }

    add_moves<false, KNIGHT>(pos, ml, idx);
}

/* Specialisation for pawn captures. */
/* (en-passant, capture-promotions) */
template<> void add_moves<true, PAWN>(const Position& pos, Move* ml, int& idx)
{
    std::uint64_t pawns = get_piece(pos, PAWN, US);
    std::uint64_t them = get_colour(pos, THEM);
    std::uint64_t dest_bb;

    // Left captures
    dest_bb = ((pawns & ~file_mask[FILE_A]) << 7) & them & ~rank_mask[RANK_8];

    while (dest_bb) {
        Square dest = lsb(dest_bb);

        ml[idx] = get_move(dest - 7, dest, CAPTURE);
        idx++;

        dest_bb &= dest_bb - 1;
    }

    // Left capture-promotions
    dest_bb = ((pawns & ~file_mask[FILE_A]) << 7) & them & rank_mask[RANK_8];

    while (dest_bb) {
        Square dest = lsb(dest_bb);

        ml[idx] = get_move(dest - 7, dest, PROM_CAPTURE, TO_KNIGHT);
        idx++;

        ml[idx] = get_move(dest - 7, dest, PROM_CAPTURE, TO_BISHOP);
        idx++;

        ml[idx] = get_move(dest - 7, dest, PROM_CAPTURE, TO_ROOK);
        idx++;

        ml[idx] = get_move(dest - 7, dest, PROM_CAPTURE, TO_QUEEN);
        idx++;

        dest_bb &= dest_bb - 1;
    }

    // Right captures
    dest_bb = ((pawns & ~file_mask[FILE_H]) << 9) & them & ~rank_mask[RANK_8];

    while (dest_bb) {
        Square dest = lsb(dest_bb);

        ml[idx] = get_move(dest - 9, dest, CAPTURE);
        idx++;

        dest_bb &= dest_bb - 1;
    }

    // Right capture-promotions
    dest_bb = ((pawns & ~file_mask[FILE_H]) << 9) & them & rank_mask[RANK_8];

    while (dest_bb) {
        Square dest = lsb(dest_bb);

        ml[idx] = get_move(dest - 9, dest, PROM_CAPTURE, TO_KNIGHT);
        idx++;

        ml[idx] = get_move(dest - 9, dest, PROM_CAPTURE, TO_BISHOP);
        idx++;

        ml[idx] = get_move(dest - 9, dest, PROM_CAPTURE, TO_ROOK);
        idx++;

        ml[idx] = get_move(dest - 9, dest, PROM_CAPTURE, TO_QUEEN);
        idx++;

        dest_bb &= dest_bb - 1;
    }

    // En passant
    if (pos.epsq != INVALID_SQUARE) {
        pawns &= pawn_attacks(pos.epsq, THEM);

        while (pawns) {
            Square from = lsb(pawns);

            ml[idx] = get_move(from, pos.epsq, ENPASSANT);
            idx++;

            pawns &= pawns - 1;
        }
    }

    add_moves<true, KNIGHT>(pos, ml, idx);
}

/* Specialisation for king quiets. */
/* (Castling, plus an end to the recursion) */
template<>
void add_moves<false, KING>(const Position& pos, Move* ml, int& idx)
{
    std::uint64_t occ = get_occupancy(pos);

    Square from = lsb(get_piece(pos, KING, US));

    std::uint64_t attack_bb = attacks<KING>(from, occ) & ~occ;

    while (attack_bb) {
        Square dest = lsb(attack_bb);

        ml[idx] = get_move(from, dest, NORMAL);
        idx++;

        attack_bb &= attack_bb - 1;
    }

    if (!is_checked(pos, US)) {
        if (pos.castle & US_OO && !(occ & oo_castle_mask) &&
            !(attacks_to<>(pos, F1, occ, US) & get_colour(pos, THEM))&&
            !(attacks_to<>(pos, G1, occ, US) & get_colour(pos, THEM))) {

            ml[idx] = get_move(E1, G1, CASTLE);
            idx++;
        }

        if (pos.castle & US_OOO && !(occ & ooo_castle_mask) &&
            !(attacks_to<>(pos, D1, occ, US) & get_colour(pos, THEM))&&
            !(attacks_to<>(pos, C1, occ, US) & get_colour(pos, THEM))) {

            ml[idx] = get_move(E1, C1, CASTLE);
            idx++;
        }
    }

    // No tail call to end template recursion.
}

/* Specialisation for king captures. */
/* Maybe worth checking for illegal moves? */
template<>
void add_moves<true, KING>(const Position& pos, Move* ml, int& idx)
{
    std::uint64_t pieces = get_piece(pos, KING, US);
    std::uint64_t occ = get_colour(pos, US) | get_colour(pos, THEM);

    Square from = lsb(pieces);

    std::uint64_t attack_bb = attacks<KING>(from, occ) & get_colour(pos, THEM);

    while (attack_bb) {
        Square dest = lsb(attack_bb);

        ml[idx] = get_move(from, dest, CAPTURE);
        idx++;

        attack_bb &= attack_bb - 1;
    }

    // No tail call to end template recursion.
}

/* Generate moves for a position. */
int generate(const Position& pos, Move* ml)
{
    int idx = 0;

    add_moves<true>(pos, ml, idx);
    add_moves<false>(pos, ml, idx);

    return idx;
}
