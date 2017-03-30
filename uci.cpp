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

#include "types.h"
#include "move.h"
#include "position.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>

/* reads from the file descriptor fd into a newly allocated buffer pointed to by buff,
 * until either it hits EOF or '\n'.
 * Returns a pointer to the null-terminated buffer, without the '\n' character at the end. */
char *read_until_newline_alloc(FILE *fd)
{
    static std::size_t const alloc_size = 127;

    assert(fd);

    char *buff = NULL;
    std::size_t idx = 0;
    std::size_t curr_size = 0;
    int c;

    //this is to skip initial whitespace (can be removed)
    while ( (c = std::fgetc(fd)) != EOF && (c == '\n') );

    if (!std::feof(fd)) {

        curr_size = alloc_size;
        buff = (char*)std::malloc(curr_size);
        assert(buff);

        do {

            if (idx >= curr_size) {

                curr_size += alloc_size;
                buff = (char*)std::realloc(buff, curr_size);
                assert(buff);

            }

            buff[idx++] = (char)c;

        } while ( ((c = std::fgetc(fd)) != EOF) && c != '\n');

        if (idx >= curr_size) {

            buff = (char*)std::realloc(buff, curr_size + 1);
            assert(buff);

        }

        buff[idx] = '\0';

    }

    return buff;
}

/* reads from the file descriptor fd into the pre-allocated buffer pointed to by buff,
 * until either it hits EOF, '\n', or it reads buff_len - 1 characters.
 * Returns true if it read less than buff_len characters (or if the EOF character was read)
 * false otherwise. */
bool read_until_newline_auto(FILE *fd, char *buff, std::size_t buff_len)
{
    assert(buff && fd && buff_len);

    std::size_t idx = 0, idx_len = buff_len - 1;
    int c;
    bool ret = true;

    while ( ((c = std::fgetc(fd)) != EOF) && c != '\n') {

        if (idx == idx_len)
            break;

        buff[idx++] = (char)c;

    }

    //flush stdin
    if (c != EOF && c != '\n') {
        ret = false;
        while ( ((c = std::fgetc(fd)) != EOF) && c != '\n');
    }

    buff[idx] = '\0';

    return ret;
}

int uci_main(int argc, char *argv[])
{
    std::puts("id name CPirc Chess-Engine\n");

    //send options to the GUI here
    //std::puts("option \n");

    std::puts("uciok\n");

    char *msg = NULL;


    //set options
    while (1) {

        msg = read_until_newline_alloc(stdin);

        if (!std::strcmp(msg, "quit")) {
            std::free(msg);
            break;
        }

        /* if (std::strstr(msg, "setoption")) {
            //handle options
        } */

        

        std::free(msg);

    }

    return 0;
}
