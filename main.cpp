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
#include <cctype>
#include <cassert>

#include "bitboard.h"
#include "uci.h"


/* reads from the file descriptor fd into the pre-allocated buffer pointed to by buff,
 * until either it hits EOF, '\n', or it reads buff_len - 1 characters.
 * Returns true if it read less than buff_len characters (or if the EOF character was read)
 * false otherwise. */
bool getline_auto(FILE *fd, char *buff, std::size_t buff_len)
{
    assert(buff && fd && buff_len);

    std::size_t idx = 0, idx_len = buff_len - 1;
    int curr;
    bool ret = true;

    while ( ((curr = std::fgetc(fd)) != EOF) && curr != '\n') {

        if (idx == idx_len) {
            break;
        }

        if (!std::isblank(curr)) {
            buff[idx++] = (char)curr;
        }
    }

    //flush stdin
    if (curr != EOF && curr != '\n') {
        ret = false;

        while ( ((curr = std::fgetc(fd)) != EOF) && curr != '\n');
    }

    buff[idx] = '\0';

    return ret;
}

/* The start of all things (after _start) */
int main(int argc, char *argv[])
{
    init_bitboards();

    char protocol[12];

    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::setvbuf(stdin, NULL, _IONBF, 0);

    if (getline_auto(stdin, protocol, 12)) {

        if (!std::strncmp(protocol, "uci", 3)) {
            return uci_main(argc, argv);
        }

        std::puts("Unsupported protocol!");
    } else
        std::puts("Stack overflow!");

    /*Position pos;
    parse_fen_to_position("3q3k/1Q4R1/2pNB2p/2Pp3n/8/6P1/3r2r1/7K b - - 3 38", pos);
    parse_fen_to_position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", pos);
    start_search(pos);*/

    return 1;
}

