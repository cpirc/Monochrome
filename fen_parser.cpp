#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unordered_map>


#define PRINT_BITBOARD(x) \
    for (int i = 0; i < 64; i++) { \
        printf("%d ", (((x) >> i) & 1) ? 1 : 0); \
        if (!((i + 1) % 8)) \
            putchar('\n'); \
    } \
    putchar('\n');

#define INVALID_SQUARE (Square)64
#define ARR_LEN(x) (sizeof(x) / sizeof(x[0]))

static std::unordered_map<char, Piece>
   b_piece_map = {
    {'p', PAWN},
    {'r', ROOK},
    {'n', KNIGHT},
    {'b', BISHOP},
    {'q', QUEEN},
    {'k', KING}
}, w_piece_map = {
    {'P', PAWN},
    {'R', ROOK},
    {'N', KNIGHT},
    {'B', BISHOP},
    {'Q', QUEEN},
    {'K', KING}
};

static std::unordered_map<char, std::size_t> castle_bit_mask = {
    {'q', 0},
    {'k', 1},
    {'Q', 2},
    {'K', 3},
};

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
    std::size_t i = 0, square_idx = 0;
    char c;

    std::memset(&pos, 0, ( sizeof(((Position*)0)->pieces) + sizeof(((Position*)0)->colours) ));

    while (square_idx < ARR_LEN(fen_board)) {

        c = fen_str[i++];

        switch (c) {
        case 'p':
        case 'r':
        case 'n':
        case 'b':
        case 'q':
        case 'k':
            //printf("square_idx = %zu, piece = %c\n",
            put_piece(pos, (const Square)fen_board[square_idx], b_piece_map[c], THEM);
            square_idx++;
            break;
        case 'P':
        case 'R':
        case 'N':
        case 'B':
        case 'Q':
        case 'K':
            put_piece(pos, (const Square)fen_board[square_idx], w_piece_map[c], US);
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
    pos.flipped = (c == 'w') ? false : true;

    i+=2;
    do {
        c = fen_str[i++];

        switch (c) {
        case 'K':
        case 'Q':
        case 'k':
        case 'q':
            pos.castle |= 1 << castle_bit_mask[c];
            break;
        case '-':
            pos.castle = 0;
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

    printf("Positions of black pieces:\n");
    PRINT_BITBOARD(pos.colours[US]);

    printf("Positions of white pieces:\n");
    PRINT_BITBOARD(pos.colours[THEM]);

    printf("Side to move is: %s\n\n", (pos.flipped == true) ? "black" : "white");

    if ((pos.castle << castle_bit_mask['K']))
        printf("White king can castle kingside\n");
    if ((pos.castle << castle_bit_mask['Q']))
        printf("White king can castle queenside\n");

    if ((pos.castle << castle_bit_mask['k']))
        printf("Black king can castle kingside\n");
    if ((pos.castle << castle_bit_mask['q']))
        printf("Black king can castle queenside\n");

    if (pos.epsq != INVALID_SQUARE)
        printf("\nEn passant square is: %u\n", (unsigned int)pos.epsq);

    printf("Number of half moves: %d\n", (int)pos.fifty);
}
/*
int main(void)
{
    Position tmp;

    initialize_keys();
    //parse_fen_to_position((const char*)"rnbqkbnr//pppppppp//8//8//8//8//PPPPPPPP//RNBQKBNR w KQkq - 0 1", tmp);
    //parse_fen_to_position((const char*)"rnbqkbnr//pppppppp//8///8//4P3//8//PPPP1PPP//RNBQKBNR b KQkq e3 0 1", tmp);
    //parse_fen_to_position((const char*)"rnbqkbnr//pp1ppppp//8//2p5//4P3//8//PPPP1PPP//RNBQKBNR w KQkq c6 0 2", tmp);
    parse_fen_to_position((const char*)"rnbqkbnr//pp1ppppp//8//2p5//4P3//5N2//PPPP1PPP//RNBQKB1R b KQkq - 1 2", tmp);
    print_position_struct(tmp);
    return 0;
}
*/
