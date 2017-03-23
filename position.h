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
#include "bitboard.h"
#include "tt.h"

/* A chess position. */
struct Position {
    std::uint64_t pieces[6];  // Bitboards containing piece locations.
    std::uint64_t colours[2]; // Bitboards containing colours of pieces.
    std::uint8_t castle;      // Castling rights.
    Square epsq;              // En passant square.
    char fifty;               // Fifty-move rule counter.
    std::uint64_t hash_key;   // Zobrist hash of the current position.
};

extern std::uint64_t side_keys[2];
extern std::uint64_t castle_keys[16];
extern std::uint64_t piece_sq_keys[2][6][64];

extern void initialize_keys();

/* Extract data from a FEN string to a Position struct */
extern void parse_fen_to_position(const char* fen_str, Position& pos);

extern void run_fen_parser_tests();

/* Get the type of piece on a square(bitboard of the square) */
template<Piece piece = PAWN>
inline Piece get_piece_on_square(const Position& pos, const std::uint64_t sq)
{
    if (pos.pieces[piece] & sq)
        return piece;
    return get_piece_on_square<piece+1>(pos, sq);
}

/* Get the type of piece on a square(bitboard of the square) */
template<>
inline Piece get_piece_on_square<KING>(const Position& pos, const std::uint64_t sq)
{
    if (pos.pieces[KING] & sq)
        return KING;
    return NO_PIECE;
}

/* Get the type of piece on a square */
template<Piece piece = PAWN>
inline Piece get_piece_on_square(const Position& pos, const Square sq)
{
    return get_piece_on_square<piece>(pos, (1ULL << sq));
}

/* Get a piece bitboard. */
inline std::uint64_t get_piece(const Position& b, const Piece p)
{
    return b.pieces[p];
}

/* Get a colour bitboard. */
inline std::uint64_t get_colour(const Position& b, const Colour c)
{
    return b.colours[c];
}

inline std::uint64_t get_occupancy(const Position& pos)
{
    return pos.colours[US] | pos.colours[THEM];
}

/* Get a piece bitboard of a colour. */
inline std::uint64_t get_piece(const Position& b, const Piece p, const Colour c)
{
    return get_piece(b, p) & get_colour(b, c);
}

/* Set a bit in a piece bitboard. */
inline void set_bit(Position& b, const Piece p, const Square sq)
{
    b.pieces[p] |= 1ULL << sq;
}

/* Set a bit in a colour bitboard. */
inline void set_bit(Position& b, const Colour c, const Square sq)
{
    b.colours[c] |= 1ULL << sq;
}

/* Clear a bit in a piece bitboard. */
inline void clear_bit(Position& b, const Piece p, const Square sq)
{
    b.pieces[p] &= ~(1ULL << sq);
}

/* Clear a bit in a colour bitboard. */
inline void clear_bit(Position& b, const Colour c, const Square sq)
{
    b.colours[c] &= ~(1ULL << sq);
}

/* Updates the position by moving piece from 'from' to 'to' */
inline void move_piece(Position& pos, const Square from, const Square to,
                      const Piece piece, const Colour colour)
{
    assert(from != to);
    std::uint64_t from_to = (1ULL << from) ^ (1ULL << to);
    pos.pieces[piece]    ^= from_to;
    pos.colours[colour]  ^= from_to;
    pos.hash_key         ^= piece_sq_keys[colour][piece][from]
                          ^ piece_sq_keys[colour][piece][to];
}

/* Updates the position by putting piece on 'to' */
inline void put_piece(Position& pos, const Square to, const Piece piece,
                      const Colour colour)
{
    std::uint64_t to_bit = (1ULL << to);
    pos.pieces[piece]   |= to_bit;
    pos.colours[colour] |= to_bit;
    pos.hash_key        ^= piece_sq_keys[colour][piece][to];
}

/* Updates the position by removing piece from 'from' */
inline void remove_piece(Position& pos, const Square from, const Piece piece,
                      const Colour colour)
{
    std::uint64_t from_bit = (1ULL << from);
    pos.pieces[piece]     ^= from_bit;
    pos.colours[colour]   ^= from_bit;
    pos.hash_key          ^= piece_sq_keys[colour][piece][from];
}

/* Get any piece attacks to a square. */
template<Piece p = PAWN>
inline std::uint64_t attacks_to(const Position& pos, const Square sq, const std::uint64_t occ, const Colour by)
{
    return (attacks<p>(sq, occ) & get_piece(pos, p)) | attacks_to<p+1>(pos, sq, occ, by);
}

template<>
inline std::uint64_t attacks_to<PAWN>(const Position& pos, const Square sq, const std::uint64_t occ, const Colour by)
{
    return (pawn_attacks(sq, by) & get_piece(pos, PAWN)) | attacks_to<KNIGHT>(pos, sq, occ, by);
}

/* Finish above template. */
template<>
inline std::uint64_t attacks_to<KING>(const Position& pos, const Square sq, const std::uint64_t occ, const Colour)
{
    return attacks<KING>(sq, occ) & get_piece(pos, KING);
}

/* Checks to see if c is in check */
inline bool is_checked(const Position& pos, const Colour c)
{
    Colour by = (c == US ? THEM : US);
    return (attacks_to(pos, lsb(get_piece(pos, KING, c)), get_occupancy(pos), c) & get_colour(pos, by)) > std::uint64_t(0);
}

/* Flips the position */
inline void flip_position(Position& pos)
{
    // Flip piece bitboards
    std::uint64_t* curr;
    for (curr = pos.pieces; curr < pos.pieces + 6; ++curr)
        *curr = __builtin_bswap64(*curr);
    std::uint64_t tmp = pos.colours[1];

    // Reverse colour bitboards
    pos.colours[1] = pos.colours[0];
    pos.colours[0] = tmp;

    // Flip colour bitboards
    for (curr = pos.colours; curr < pos.colours + 2; ++curr)
        *curr = __builtin_bswap64(*curr);

    // Flip epsq
    if (pos.epsq != INVALID_SQUARE)
        pos.epsq = Square(int(pos.epsq) ^ 56);

    // Flip castling rights
    std::uint8_t tmp2 = (pos.castle & 3) << 2;
    pos.castle >>= 2;
    pos.castle ^= tmp2;
}

extern std::uint64_t perft(const Position& pos, int depth);
extern void run_perft_tests();

#endif
