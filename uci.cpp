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
        std::printf("=== "); \
        std::printf(fmt, ##__VA_ARGS__); \
        std::printf(" ===\n"); \
    } while (0)
#else
# define LOG(fmt, ...)
#endif


struct EngineOptions {
    bool debug;
    bool registration;
    /* ... */
} opt = {
    .debug = false, //debugging is turned off, by default
    .registration = false //this engine doesn't need registration to fully work
};

static Position pos;

//stores the last character that was read from stdin
//this is a global, because functions need to know the last
//char in order to do proper line buffering
static int c;

static bool running;

static void handle_eof();

static void flush_up_to_char(int upto);
static void flush_up_to_whitespace();

static void handle_debug();
static void handle_go();

static void handle_position();
static void handle_position_startpos();
static void handle_position_fen();

static void handle_register();
static void handle_setoption();
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

void handle_eof()
{
    //LOG("Received EOF!");
    std::exit(EXIT_FAILURE);
}

void flush_up_to_char(int upto)
{
    do {
        c = std::fgetc(stdin);

        if (c == EOF)
            handle_eof();

    } while (c != upto);
}

void flush_up_to_whitespace()
{
    do {
        c = std::fgetc(stdin);

        if (c == EOF)
            handle_eof();

    } while (!std::isspace(c));
}

void handle_debug()
{
    std::size_t i = 0;
    char s[4]; //the debug command can only be followed by
    //the strings "on" or "off". s has the length of the biggest
    //allowed string, plus '\0' (size of "off").

    while (1) {

        c = std::fgetc(stdin);

        if (c == EOF) {

            handle_eof();

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
                    LOG("Unrecognized token : \"%s\"", s);
                }

                //now that we processed this command
                //we skip up to the first whitespace
                //before returning (if we haven't hit whitespace already)
                if (c != '\n') {
                    flush_up_to_char('\n');
                }

                break;
            }

            if (c == '\n') {
                LOG("Invalid use of debug command");
                break;
            }

        } else {

            if (i >= 3) {
                flush_up_to_char('\n');

                s[3] = 0;

                LOG("Token \"%s...\" exceeds max token length", s);
                break;
            }

            s[i++] = (char)c;

        }
    }
}

void handle_go()
{
    flush_up_to_char('\n');
}

void handle_position_fen()
{
    
}

void handle_position_startpos()
{
    
}

void handle_position()
{
    std::size_t i = 0;
    char s[9];

    while (1) {

        c = std::fgetc(stdin);

        if (c == EOF) {

            handle_eof();

        } else if (std::isspace(c)) {

            if (i) {

                s[i] = 0;

                if (!std::strcmp(s, "fen")) {
                    LOG("fen format");
                    handle_position_fen();
                } else if (!std::strcmp(s, "startpos")) {
                    LOG("lan format");
                    handle_position_startpos();
                } else {
                    LOG("Unrecognized token : \"%s\"", s);
                }

                //now that we processed this command
                //we skip up to the first whitespace
                //before returning (if we haven't hit whitespace already)
                if (c != '\n') {
                    flush_up_to_char('\n');
                }

                break;
            }

            if (c == '\n') {
                LOG("Invalid use of position command");
                break;
            }

        } else {

            if (i >= 8) {
                flush_up_to_char('\n');

                s[8] = 0;

                LOG("Token \"%s...\" exceeds max token length", s);
                break;
            }

            s[i++] = (char)c;

        }
    }
}

void handle_register()
{
    flush_up_to_char('\n');
}

void handle_setoption()
{
    flush_up_to_char('\n');
}

void handle_isready()
{
    std::puts("readyok");
}

void handle_ucinewgame()
{
    std::memset((void*)&pos, 0, sizeof(Position));
}

void handle_stop()
{
}

void handle_ponderhit()
{
}

void handle_quit()
{
    running = false;
}

//seperate handler for single word commands that aren't
//bound to be followed by other words (e.g "isready\n", "    \t\t  ucinewgame\n")
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
    } else if (!std::strcmp(cmd, "quit")) {
        LOG("quit command");
        handle_quit();
    } else {
        LOG("Unrecognized token : \"%s\"", cmd);
    }
}

//returns false if the command that was about to be processed
//was a simple command and was followed by characters other than whitespace
//e.g. "ucinewgame       test\n"
//returns true on all other cases
bool handle_all_commands(char *cmd)
{
    if (!std::strcmp(cmd, "debug")) {
        LOG("debug command");
        handle_debug();
    } else if (!std::strcmp(cmd, "go")) {
        LOG("go command");
        handle_go();
    } else if (!std::strcmp(cmd, "position")) {
        LOG("position command");
        handle_position();
    } else if (opt.registration && !std::strcmp(cmd, "register")) {
        LOG("register command");
        handle_register();
    } else if (!std::strcmp(cmd, "setoption")) {
        LOG("setoption command");
        handle_setoption();
    } else {

        //this next loop cleans up the rest of the buffer until '\n'.
        //This is done, because simple commands aren't followed by options, so
        //while clearing up the buffer we check if we hit a character that isn't
        //whitespace, in which case the command processing fails.
        //e.g. legal : " isready   \t \t  \n"
        //   illegal : "\t\t isready   \t testtest \t\n"
        if (c != '\n') {

            do {
                c = std::fgetc(stdin);

                if (!std::isspace(c)) {
                    return false;
                }

                if (c == EOF)
                    handle_eof();

            } while (c != '\n');

        }

        handle_simple_commands(cmd);

    }

    return true;
}

int uci_main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    std::puts("id name CPirc Chess-Engine");
    std::puts("id author mkchan ZirconiumX Gikoskos");

    //send options to the GUI here
    //std::puts("option \n");

    std::puts("uciok");

    char token[MAX_UCICMD_LEN];
    std::size_t i = 0;

    running = true;

    //this loop, parses only the first word of a command,
    //e.g. on the string "hello world\n", this loop will stop at "hello"
    //and send that message to the handlers for processing. If "hello" is
    //a valid command, then the handlers will skip the rest of the line
    //(or they will read from it if they need more options) and return to the loop.
    //If it's an invalid command, the handlers won't do anything to this token and
    //return back to the loop, where the next token will be read from the line (if
    //there is one) and the process will repeat. If the rest of the line is just
    //whitespace characters, it will be ignored.
    while (running) { //the running bool is toggled only if the GUI sends a "quit" command

        c = std::fgetc(stdin);

        if (c == EOF) {
            handle_eof();
        } else if (c == '\n') {

            //handles cases where the word is followed by a newline like
            // "     isready\n"
            // "\t\tstop\n"
            //if a word is followed by a newline, during the parsing in this loop
            //it can't be a command that is followed by options (e.g. "debug off").
            //so we don't account for these cases
            if (i) {
                token[i] = 0;

                i = 0;

                //if the last character that was entered, was a '\n'
                //there's no need to compare the 'msg' string against
                //commands that are followed by options. We only check
                //if 'msg' is one of the single word commands, and if
                //it's not, then it's an invalid command
                handle_simple_commands(token);

            }

        } else if (std::isspace(c)) {

            //handles all other cases
            if (i) {
                token[i] = 0;

                i = 0;

                if (!handle_all_commands(token)) {
                    token[i++] = c;
                }

            }

        } else {

            //if there's a word in the buffer, that's longer
            //than the longest UCI command length (ucinewgame)
            //meaning that it's an invalid command,
            //skip all the characters up to the first whitespace
            if (i >= MAX_UCICMD_LEN) {

                i = 0;

                flush_up_to_whitespace();

                token[MAX_UCICMD_LEN - 1] = '\0';
                LOG("Token \"%s...\" exceeds max token length", token);

            } else {
                token[i++] = (char)c;
            }

        }

    }

    LOG("Quitting!");

    return 0;
}
