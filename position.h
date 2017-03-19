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
    assert(is_ok(p));

    return b.pieces[int(p)];
}

/* Get a colour bitboard. */
static inline std::uint64_t get_colour(const Position& b, const Colour c)
{
    assert(is_ok(c));

    return b.colours[int(c)];
}

/* Get a piece bitboard of a colour. */
static inline std::uint64_t get_piece(const Position& b, const Piece p, const Colour c)
{
    assert(is_ok(p));
    assert(is_ok(c));

    return get_piece(b, p) & get_colour(b, c);
}

/* Set a bit in a piece bitboard. */
static inline void set_bit(Position& b, const Piece p, const Square sq)
{
    assert(is_ok(p));
    assert(is_ok(sq));

    b.pieces[int(p)] |= 1ULL << sq;
}

/* Set a bit in a colour bitboard. */
static inline void set_bit(Position& b, const Colour c, const Square sq)
{
    assert(is_ok(c));
    assert(is_ok(sq));

    b.colours[int(c)] |= 1ULL << sq;
}

/* Clear a bit in a piece bitboard. */
static inline void clear_bit(Position& b, const Piece p, const Square sq)
{
    assert(is_ok(p));
    assert(is_ok(sq));

    b.pieces[int(p)] &= ~(1ULL << sq);
}

/* Clear a bit in a colour bitboard. */
static inline void clear_bit(Position& b, const Colour c, const Square sq)
{
    assert(is_ok(c));
    assert(is_ok(c));

    b.colours[int(c)] &= ~(1ULL << sq);
}

#endif
