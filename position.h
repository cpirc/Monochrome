#ifndef POSITION_H
#define POSITION_H

#include <cassert>
#include <cinttypes>

/* Pieces on a chessboard. */
enum Piece {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
};

/* Colours on a chessboard: */
enum Colour {
    US, THEM
};

/* A chess position. */
struct Position {
    std::uint64_t pieces[6];  // Bitboards containing piece locations.
    std::uint64_t colours[2]; // Bitboards containing colours of pieces.
    Colour side;              // Side to move.
    std::uint8_t castle;      // Castling rights.
    char epsq;                // En passant square.
    char fifty;               // Fifty-move rule counter.
    std::uint64_t hash_key;   // Zobrist hash of the current position.
};

/* Check piece validity. */
static inline bool piece_is_ok(const Piece p)
{
    return (p >= PAWN && p <= KING);
}

/* Check colour validity. */
static inline bool colour_is_ok(const Colour c)
{
    return (c == US || c == THEM);
}

/* Check square validity. */
static inline bool square_is_ok(const int sq)
{
    return (sq >= 0 && sq <= 63);
}

/* Get a piece bitboard. */
static inline std::uint64_t get_piece(const Position& b, const Piece p)
{
    assert(piece_is_ok(p));

    return b.pieces[int(p)];
}

/* Get a colour bitboard. */
static inline std::uint64_t get_colour(const Position& b, const Colour c)
{
    assert(colour_is_ok(c));

    return b.colours[int(c)];
}

/* Get a piece bitboard of a colour. */
static inline std::uint64_t get_piece(const Position& b, const Piece p, const Colour c)
{
    assert(piece_is_ok(p));
    assert(colour_is_ok(c));

    return get_piece(b, p) & get_colour(b, c);
}

/* Set a bit in a piece bitboard. */
static inline void set_bit(Position& b, const Piece p, const int sq)
{
    assert(piece_is_ok(p));
    assert(square_is_ok(sq));

    b.pieces[int(p)] |= 1ULL << sq;
}

/* Set a bit in a colour bitboard. */
static inline void set_bit(Position& b, const Colour c, const int sq)
{
    assert(colour_is_ok(c));
    assert(square_is_ok(sq));

    b.colours[int(c)] |= 1ULL << sq;
}

/* Clear a bit in a piece bitboard. */
static inline void clear_bit(Position& b, const Piece p, const int sq)
{
    assert(piece_is_ok(p));
    assert(square_is_ok(sq));

    b.pieces[int(p)] &= ~(1ULL << sq);
}

/* Clear a bit in a colour bitboard. */
static inline void clear_bit(Position& b, const Colour c, const int sq)
{
    assert(colour_is_ok(c));
    assert(square_is_ok(c));

    b.colours[int(c)] &= ~(1ULL << sq);
}

#endif
