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

#include <cinttypes>

#include "eval.h"
#include "position.h"
#include "psqt.h"
#include "types.h"

/* Piece values for material in centipawns. */
const int piecevals[2][7] = {{100, 300, 300, 500, 900, 0, 0},
                             {100, 300, 300, 500, 900, 0, 0}};

/* Phase weights for material. */
const int phase_weights[7] = {0, 1, 1, 2, 4, 0, 0};

/* Mobility weights for material. */
const int mobility_weights[7] = {0, 4, 4, 4, 4, 0, 0};

/* Passed pawn advancement bonus. */
const int passed_pawn_bonus[8] = {0, 0, 10, 20, 40, 80, 160, 0};

/* Return material balance of a board. */
/* TODO: incremental update this. */
template <Piece p = PAWN>
int evaluate_material(const Position& pos) {
    return popcnt(get_piece(pos, p, US)) * piecevals[OPENING][p] +
           evaluate_material<p + 1>(pos);
}

template <>
inline int evaluate_material<QUEEN>(const Position& pos) {
    return popcnt(get_piece(pos, QUEEN, US)) * piecevals[OPENING][QUEEN];
}

/* Return material phase of a board. */
/* 24 = opening, 0 = endgame. */
/* TODO: incremental update this. */
template <Piece p = PAWN>
int material_phase(const Position& pos) {
    std::uint64_t pieces = get_piece(pos, p, US);

    return (popcnt(pieces) * phase_weights[p]) +
           (p == KING ? 0 : material_phase<p + 1>(pos));
}

/* PST values for the pieces on the board. */
template <Phase ph, Piece p = PAWN>
inline int evaluate_pst(const Position& pos) {
    int score = 0;
    uint64_t pieces = get_piece(pos, p, US);

    while (pieces) {
        score += pst[p][ph][lsb(pieces)];

        pieces &= pieces - 1;
    }

    return score + evaluate_pst<ph, p + 1>(pos);
}

template <>
inline int evaluate_pst<OPENING, KING>(const Position& pos) {
    int score = 0;
    uint64_t pieces = get_piece(pos, KING, US);

    while (pieces) {
        score += pst[KING][OPENING][lsb(pieces)];

        pieces &= pieces - 1;
    }

    return score;
}

template <>
inline int evaluate_pst<ENDGAME, KING>(const Position& pos) {
    int score = 0;
    uint64_t pieces = get_piece(pos, KING, US);

    while (pieces) {
        score += pst[KING][ENDGAME][lsb(pieces)];

        pieces &= pieces - 1;
    }

    return score;
}

inline int king_safety(const Position& pos) {
    std::uint64_t king_bb = get_piece(pos, KING, US);
    Square king_sq = lsb(king_bb);
    std::uint64_t surrounding = attacks<KING>(king_sq, (std::uint64_t)0);

    int score = 0;

    // Nearby friendly pieces
    score += 5 * popcnt(surrounding & get_colour(pos, US));

    // Nearby unfriendly pieces
    // score -= 5*popcnt(surrounding & get_colour(pos, THEM));

    return score;
}

/* Evaluate piece mobility. */
template <Piece p = KNIGHT>
int evaluate_mobility(const Position& pos) {
    int score = 0;
    uint64_t pieces = get_piece(pos, p, US);

    while (pieces) {
        score += popcnt(attacks<p>(lsb(pieces), get_occupancy(pos))) *
                 mobility_weights[p];

        pieces &= pieces - 1;
    }

    return score + evaluate_mobility<p + 1>(pos);
}

template <>
int evaluate_mobility<QUEEN>(const Position& pos) {
    int score = 0;
    uint64_t pieces = get_piece(pos, QUEEN, US);

    while (pieces) {
        score += popcnt(attacks<QUEEN>(lsb(pieces), get_occupancy(pos))) *
                 mobility_weights[QUEEN];

        pieces &= pieces - 1;
    }

    return score;
}

inline int evaluate_passers(const Position& pos) {
    int score = 0;
    std::uint64_t pawns = get_piece(pos, PAWN, US);

    while (pawns) {
        int sq = lsb(pawns);
        int rank = sq >> 3;
        int file = sq & 7;  // Do we have a macro for this?

        std::uint64_t mask = 0;

        mask |= 0x0101010101010101ULL << (sq + 8);

        if (file >= 1) {
            mask |= 0x0101010101010101ULL << (sq + 7);
        }

        if (file <= 6) {
            mask |= 0x0101010101010101ULL << (sq + 9);
        }

        if (!(mask & get_piece(pos, PAWN, THEM))) {
            score += passed_pawn_bonus[rank];
        }

        pawns &= pawns - 1;
    }

    return score;
}

/* Return the heuristic value of a position. */
int evaluate(Position& pos) {
    int opening = 0, endgame = 0;
    int phase = material_phase<>(pos);
    Colour side;

    for (side = US; side <= THEM; ++side) {
        // material + PST
        opening += evaluate_material<>(pos) + evaluate_pst<OPENING>(pos);
        endgame += evaluate_material<>(pos) + evaluate_pst<ENDGAME>(pos);

        // King safety
        opening += king_safety(pos);

        opening += evaluate_mobility<>(pos);
        endgame += evaluate_mobility<>(pos);

        // Passed pawns
        opening += evaluate_passers(pos);
        endgame += evaluate_passers(pos);

        opening = -opening;
        endgame = -endgame;
        flip_position(pos);
    }

    int score = ((phase * opening) + ((24 - phase) * endgame)) / 24;

    return score;
}
