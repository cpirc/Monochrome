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
#include <cctype> //for std::isspace


#define MAX_UCICMD_LEN 11

#define ENABLE_LOGGING //comment this line out to disable log messages

#ifdef ENABLE_LOGGING
# define LOG(fmt, ...) \
    do { \
        std::printf(fmt, ##__VA_ARGS__); \
        std::putchar('\n'); \
    } while (0)
#else
# define LOG(fmt, ...)
#endif


struct EngineOptions {
    bool debug;
    /* ... */
} opt = {
    .debug = false //debugging is turned off, by default
};

static Position pos;

//stores the last character that was read from stdin
//this is a global, because functions need to know the last
//char in order to do proper line buffering
static int c;


static bool flush_up_to_char(int upto);
static bool flush_up_to_whitespace();

static bool handle_debug();
static bool handle_go();
static bool handle_position();
static bool handle_register();
static bool handle_setoption();
static void handle_isready();
static void handle_ucinewgame();
static void handle_stop();
static void handle_ponderhit();

static void handle_simple_commands(char *cmd);
static bool handle_all_commands(char *cmd);


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

        if (idx == idx_len)
            break;

        buff[idx++] = (char)curr;

    }

    //flush stdin
    if (curr != EOF && curr != '\n') {
        ret = false;
        while ( ((curr = std::fgetc(fd)) != EOF) && curr != '\n');
    }

    buff[idx] = '\0';

    return ret;
}

bool flush_up_to_char(int upto)
{
    do {
        c = std::fgetc(stdin);

        if (c == EOF)
            return false;

    } while (c != upto);

    return true;
}

bool flush_up_to_whitespace()
{
    do {
        c = std::fgetc(stdin);

        if (c == EOF)
            return false;

    } while (!std::isspace(c));

    return true;
}

//returns false on EOF
//true on all other cases
//debug command handler
bool handle_debug()
{
    std::size_t i = 0;
    char s[4];

    while (1) {

        c = std::fgetc(stdin);

        if (c == EOF) {

            return false;

        } else if (std::isspace(c)) {

            if (i) {
                s[i] = 0;

                if (!std::strcmp(s, "off")) {
                    LOG("debug was disabled");
                    opt.debug = false;
                } else if (!std::strcmp(s, "on")) {
                    LOG("debug was enabled");
                    opt.debug = true;
                } else {
                    LOG("Unknown command!");
                }

                //now that we processed this command
                //we skip up to the first whitespace
                //before returning (if we haven't hit whitespace already)
                if (c != '\n') {
                    return flush_up_to_char('\n');
                }

                return true;
            }

            if (c == '\n') {
                return true;
            }

        } else {

            if (i >= 4) {
                LOG("Unknown command!");
                return flush_up_to_char('\n');
            }

            s[i++] = (char)c;

        }
    }

    return true;
}

bool handle_go()
{
    return flush_up_to_char('\n');
}

bool handle_position()
{
    return flush_up_to_char('\n');
}

bool handle_register()
{
    return flush_up_to_char('\n');
}

bool handle_setoption()
{
    return flush_up_to_char('\n');
}

void handle_isready()
{
}

void handle_ucinewgame()
{
}

void handle_stop()
{
}

void handle_ponderhit()
{
}

//seperate handler for single word commands that aren't
//bound to be followed by other words
void handle_simple_commands(char *cmd)
{
    if (!std::strcmp(cmd, "isready")) {
        LOG("isready command");
        handle_isready();
    } else if (!std::strcmp(cmd, "ucinewgame")) {
        LOG("ucinewgame command");
        handle_ucinewgame();
    } else if (!std::strcmp(cmd, "stop")) {
        LOG("stop command");
        handle_stop();
    } else if (!std::strcmp(cmd, "ponderhit")) {
        LOG("ponderhit command");
        handle_ponderhit();
    } else {
        LOG("Unknown command : %s!", cmd);
    }
}

//returns false if EOF was hit while processing a command
//returns true on all other cases (even if the command was wrong)
bool handle_all_commands(char *cmd)
{
    if (!std::strcmp(cmd, "debug")) {
        LOG("debug command");
        return handle_debug();
    }

    if (!std::strcmp(cmd, "go")) {
        LOG("go command");
        return handle_go();
    }

    if (!std::strcmp(cmd, "position")) {
        LOG("position command");
        return handle_position();
    }

    if (!std::strcmp(cmd, "register")) {
        LOG("register command");
        return handle_register();
    }

    if (!std::strcmp(cmd, "setoption")) {
        LOG("setoption command");
        return handle_setoption();
    }

    //handles "arbitrary white space between tokens is allowed" case
    //for single-word commands. if other characters are found after a command
    //it returns without processing it.
    //e.g. legal : " isready   \t \t  \n"
    //   illegal : "\t\t isready   \t testtest \t\n"
    if (c != '\n') {

        do {
            c = std::fgetc(stdin);

            if (!std::isspace(c)) {
                //LOG("Unknown command : %s!", cmd);
                return true;
            }

            if (c == EOF)
                return false;

        } while (c != '\n');

    }

    handle_simple_commands(cmd);

    return true;
}


//returns 1 if it reads EOF from stdin
//returns 0 on all other cases
int uci_main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    std::puts("id name CPirc Chess-Engine");
    std::puts("id author mkchan ZirconiumX Gikoskos");

    //send options to the GUI here
    //std::puts("option \n");

    std::puts("uciok");

    char msg[MAX_UCICMD_LEN];
    std::size_t i = 0;


    //this loop, parses only the first word of a command,
    //e.g. on the string "hello world\n", this loop will stop at "hello"
    //and send that message to the handlers for processing. If "hello" is
    //a valid command, then the handlers will skip the rest of the line
    //(or they will read from it if they need more options) and return to the loop.
    //If it's an invalid command, the handlers won't do anything to this token and
    //return back to the loop, where the next token will be read from the line (if
    //there is one) and the process will repeat. If the rest of the line is just
    //whitespace characters, it will be ignored.
    while (1) {

        c = std::fgetc(stdin);

        if (c == EOF) {
            return 1;
        } else if (c == '\n') {

            //handles cases where the word is followed by a newline like
            // "     isready\n"
            // "\t\tstop\n"
            //if a word is followed by a newline, during the parsing in this loop
            //it can't be a command that is followed by options (e.g. "debug off").
            //so we don't account for these cases
            if (i) {
                msg[i] = 0;

                if (!std::strcmp(msg, "quit")) {
                    LOG("quitting!!!");
                    break;
                }

                //if the last character that was entered, was a '\n'
                //there's no need to compare the 'msg' string against
                //commands that are followed by options. We only check
                //if 'msg' is one of the single word commands, and if
                //it's not, then it's an invalid command
                handle_simple_commands(msg);

                i = 0;
            }

        } else if (std::isspace(c)) {

            //handles all other cases
            if (i) {
                msg[i] = 0;

                if (!std::strcmp(msg, "quit")) {
                    LOG("quitting!!!");
                    break;
                }

                //if the command was processed correctly
                //then handle_all_commands will skip up to the first newline ('\n')
                //character, meaning that the next char that will be read
                //will be the first char on the next line (or on the next command)
                if (!handle_all_commands(msg))
                    return 1;

                i = 0;

                if (!std::isspace(c)) {
                    msg[i++] = c;
                }
            }

        } else {

            //if there's a word in the buffer, that's longer
            //than the longest UCI command length (ucinewgame)
            //meaning that it's an invalid command,
            //skip all the characters up to the first whitespace
            if (i >= MAX_UCICMD_LEN) {

                i = 0;

                if (!flush_up_to_whitespace())
                    return 1;

            }

            msg[i++] = (char)c;

        }

    }

    return 0;
}
