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

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "types.h"
#include "bitboard.h"
#include "position.h"

/* The zobrist keys used to hash the position */
std::uint64_t piece_sq_keys[2][6][64];
std::uint64_t castle_keys[16];
std::uint64_t side_keys[2];

/* Initialize the zobrist keys */
void initialize_keys()
{
    int i, j, k;
    for (i = 0; i < 2; ++i) {
        side_keys[i] = get_rand64();
        for (j = 0; j < 16; ++j) {
            castle_keys[j] = get_rand64();
        }
        for (j = 0; j < 6; ++j) {
            for (k = 0; k < 64; ++k) {
                piece_sq_keys[i][j][k] = get_rand64();
            }
        }
    }
}


#define ARR_LEN(x) (sizeof(x) / sizeof(x[0]))

static const unsigned char fen_board[] = {
    A8, B8, C8, D8, E8, F8, G8, H8,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A1, B1, C1, D1, E1, F1, G1, H1
};

void parse_fen_to_position(const char *fen_str, Position &pos)
{
    printf("Converting %s\n", fen_str);
    std::size_t i = 0, square_idx = 0;
    char c;

    std::memset(&pos, 0, ( sizeof(((Position*)0)->pieces) + sizeof(((Position*)0)->colours) ));

    while (square_idx < ARR_LEN(fen_board)) {

        c = fen_str[i++];

        switch (c) {
        case 'p':
            put_piece(pos, (const Square)fen_board[square_idx], PAWN, THEM);
            square_idx++;
            break;
        case 'r':
            put_piece(pos, (const Square)fen_board[square_idx], ROOK, THEM);
            square_idx++;
            break;
        case 'n':
            put_piece(pos, (const Square)fen_board[square_idx], KNIGHT, THEM);
            square_idx++;
            break;
        case 'b':
            put_piece(pos, (const Square)fen_board[square_idx], BISHOP, THEM);
            square_idx++;
            break;
        case 'q':
            put_piece(pos, (const Square)fen_board[square_idx], QUEEN, THEM);
            square_idx++;
            break;
        case 'k':
            put_piece(pos, (const Square)fen_board[square_idx], KING, THEM);
            square_idx++;
            break;
        case 'P':
            put_piece(pos, (const Square)fen_board[square_idx], PAWN, US);
            square_idx++;
            break;
        case 'R':
            put_piece(pos, (const Square)fen_board[square_idx], ROOK, US);
            square_idx++;
            break;
        case 'N':
            put_piece(pos, (const Square)fen_board[square_idx], KNIGHT, US);
            square_idx++;
            break;
        case 'B':
            put_piece(pos, (const Square)fen_board[square_idx], BISHOP, US);
            square_idx++;
            break;
        case 'Q':
            put_piece(pos, (const Square)fen_board[square_idx], QUEEN, US);
            square_idx++;
            break;
        case 'K':
            put_piece(pos, (const Square)fen_board[square_idx], KING, US);
            square_idx++;
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
            square_idx += (c - '0');
            break;
        }
    }

    c = fen_str[++i];
    bool flipped = c == 'b';

    i+=2;
    pos.castle = 0;
    do {
        c = fen_str[i++];

        switch (c) {
        case 'K':
            pos.castle |= US_OO;
            break;
        case 'Q':
            pos.castle |= US_OOO;
            break;
        case 'k':
            pos.castle |= THEM_OO;
            break;
        case 'q':
            pos.castle |= THEM_OOO;
            break;
        case '-':
            c = fen_str[i++];
            break;
        }

    } while (c != ' ');

    switch (fen_str[i]) {
    case '-':
        pos.epsq = INVALID_SQUARE;
        i += 2;
        break;
    default:
        pos.epsq = (Square)( (fen_str[i] - 'a') + ( (fen_str[i + 1] - '1') * 8 ) );
        i += 3;
        break;
    }

    if (fen_str[i + 1] != ' ') {
        pos.fifty = (fen_str[i] - '0')* 10 +  (fen_str[i + 1] - '1');
    } else {
        pos.fifty = fen_str[i] - '0';
    }

    pos.flipped = false;
    if (flipped)
        flip_position(pos);
}

void print_position_struct(const Position &pos)
{
    const char *piece_name[] = {
        "PAWN", "KNIGHT", "BISHOP", "ROOK", "QUEEN", "KING"
    };

    for (unsigned char p = PAWN; p <= KING; p++) {
        printf("%s positions:\n", piece_name[p]);
        PRINT_BITBOARD(pos.pieces[p]);
    }

    printf("Positions of our pieces:\n");
    PRINT_BITBOARD(pos.colours[US]);

    printf("Positions of their pieces:\n");
    PRINT_BITBOARD(pos.colours[THEM]);

    //printf("Side to move is: %s\n\n", (pos.flipped == true) ? "THEM" : "US");

    if ((pos.castle & (1 << 3)))
        printf("White king can castle kingside\n");
    if ((pos.castle & (1 << 2)))
        printf("White king can castle queenside\n");

    if ((pos.castle & (1 << 1)))
        printf("Black king can castle kingside\n");
    if ((pos.castle & (1 << 0)))
        printf("Black king can castle queenside\n");

    if (pos.epsq != INVALID_SQUARE)
        printf("\nEn passant square is: %u\n", (unsigned int)pos.epsq);

    printf("Number of half moves: %d\n", (int)pos.fifty);
}

void run_fen_parser_tests()
{
    Position tmp;

    initialize_keys();
    parse_fen_to_position((const char*)"rnbqkbnr//pppppppp//8//8//8//8//PPPPPPPP//RNBQKBNR w KQkq - 0 1", tmp);
    print_position_struct(tmp);
    parse_fen_to_position((const char*)"rnbqkbnr//pppppppp//8///8//4P3//8//PPPP1PPP//RNBQKBNR b KQkq e3 0 1", tmp);
    print_position_struct(tmp);
    parse_fen_to_position((const char*)"rnbqkbnr//pp1ppppp//8//2p5//4P3//8//PPPP1PPP//RNBQKBNR w KQkq c6 0 2", tmp);
    print_position_struct(tmp);
    parse_fen_to_position((const char*)"rnbqkbnr//pp1ppppp//8//2p5//4P3//5N2//PPPP1PPP//RNBQKB1R b Qkq - 1 2", tmp);
    print_position_struct(tmp);
}
