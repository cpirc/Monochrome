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
#include <cstring>

#include "bitboard.h"
#include "move.h"
#include "position.h"
#include "types.h"
#include "misc.h"
#include "tt.h"
#include "search.h"
#include "uci.h"


/* The start of all things (after _start) */
int main(int argc, char *argv[])
{
    init_bitboards();

    char *input = read_until_newline_alloc(stdin);

    if (!std::strcmp(input, "uci\n")) {
        free(input);
        return uci_main(argc, argv);
    }

    /*Position pos;
    parse_fen_to_position("3q3k/1Q4R1/2pNB2p/2Pp3n/8/6P1/3r2r1/7K b - - 3 38", pos);
    parse_fen_to_position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", pos);
    start_search(pos);*/

    free(input);
    std::printf("Unknown protocol\n");
    return 1;
}

