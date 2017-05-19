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
#include "bitboard.h"
#include "search.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <cerrno>
#include <cctype> //for std::isspace
#include <thread> //for thread


#define ENABLE_LOGGING //comment this line out to disable log messages
#define SEARCH_MOVES_MAX 256

#ifdef ENABLE_LOGGING
# define LOG(fmt, ...) \
    do { \
        std::printf("info debug "); \
        std::printf(fmt, ##__VA_ARGS__); \
        std::printf("\n"); \
    } while (0)
#else
# define LOG(fmt, ...)
#endif


struct EngineOptions {
    bool debug;
    bool infinite;
} opt = {
    false, //debugging is turned off, by default
    false
};

static SearchController sc;
static Move sm[SEARCH_MOVES_MAX]; //search moves
static unsigned int sm_total; //total number of search moves

//stores the last character that was read from stdin
//this is a global, because functions need to know the last
//char in order to do proper line buffering
static int c;

static bool running; //while this is true, the game loop is running

static void handle_eof();
static void send_cmd(const char *cmd);

static void flush_up_to_char(int upto);
static void flush_up_to_whitespace();
static bool flush_whitespace();

static bool read_next_ulong(unsigned long long &x);

static void handle_debug();

static void handle_go();

static void handle_position();
static void handle_position_fen();
static void handle_position_moves();

//static void handle_register();
static void handle_setoption();
static void handle_isready();
static void handle_ucinewgame();
static void handle_stop();
static void handle_ponderhit();

static void handle_simple_commands(char *cmd);
static bool handle_all_commands(char *cmd);


void handle_eof()
{
    //LOG("Received EOF!");
    std::exit(EXIT_FAILURE);
}

void send_cmd(const char *cmd)
{
    std::puts(cmd);
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

bool read_next_ulong(unsigned long long &x)
{
    static const std::size_t arr_size = 40;

    if (c == '\n' || !flush_whitespace()) {
        return false;
    }

    std::size_t i = 0;
    char ul_str[arr_size];

    do {

        if (i >= arr_size - 1) {
            flush_up_to_whitespace();
            return false;
        }

        if (!std::isdigit(c)) {
            flush_up_to_whitespace();
            return false;
        }

        ul_str[i++] = (char)c;

        c = std::fgetc(stdin);

    } while (!std::isspace(c));

    ul_str[i] = '\0';

    errno = 0;
    x = std::strtoull(ul_str, NULL, 10);
    if (errno) {
        int err = errno;
        LOG("strtoull failed with error \"%s\"", strerror(err));
        return false;
    }

    return true;
}

//skips whitespace that isn't the newline character
//returns false if '\n' was hit
//true if it wasn't hit
bool flush_whitespace()
{
    do {
        c = std::fgetc(stdin);

        if (c == EOF)
            handle_eof();

        if (c == '\n')
            return false;
    } while (std::isspace(c));

    return true;
}

void handle_debug()
{
    static const std::size_t MAX_UCICMD_LEN = 4;

    std::size_t i = 0;
    char s[MAX_UCICMD_LEN]; //the debug command can only be followed by
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

            if (i >= (MAX_UCICMD_LEN - 1)) {
                flush_up_to_char('\n');

                s[MAX_UCICMD_LEN - 1] = 0;

                LOG("Token \"%s...\" exceeds max token length", s);
                break;
            }

            s[i++] = (char)c;

        }
    }
}

void handle_go()
{
    static const std::size_t MAX_UCICMD_LEN = 12;

    std::size_t i = 0;
    char s[MAX_UCICMD_LEN];
    unsigned long long tmp;
    Move m;
    bool parse_searchmoves = false;

    sc.max_depth = MAX_PLY;
    sc.moves_per_session = 0;
    sc.increment = 0;
    sc.search_start_time = 0;
    sc.search_end_time = 0;
    sc.movetime = 0;

    sm_total = 0;

    while (1) {

        c = std::fgetc(stdin);

        if (c == EOF) {

            handle_eof();

        } else if (std::isspace(c)) {

            if (i) {

                s[i] = 0;

                if (parse_searchmoves) {

                    if ((i <= 5) && (sm_total < SEARCH_MOVES_MAX) && lan_to_move(sc.pos, s, m)) {

                        LOG("searchmoves[%u] : %s", sm_total, s);

                        sm[sm_total++] = m;

                        if (c == '\n') {
                            break;
                        }

                        i = 0;

                        continue;

                    }

                    parse_searchmoves = false;
                }

                if (!std::strcmp(s, "searchmoves")) {

                    LOG("searchmoves command");

                    parse_searchmoves = true;

                } else if (!std::strcmp(s, "ponder")) {

                    LOG("ponder command");

                } else if (!std::strcmp(s, "wtime")) {

                    if (!read_next_ulong(tmp)) {
                        LOG("Incorrect use of wtime command");
                    } else {
                        LOG("wtime = %llu", tmp);

                        if (!sc.pos.flipped)
                           sc.our_clock = tmp;
                    }

                } else if (!std::strcmp(s, "btime")) {

                    if (!read_next_ulong(tmp)) {
                        LOG("Incorrect use of btime command");
                    } else {
                        LOG("btime = %llu", tmp);

                        if (sc.pos.flipped)
                           sc.our_clock = tmp;
                    }

                } else if (!std::strcmp(s, "winc")) {

                    if (!read_next_ulong(tmp)) {
                        LOG("Incorrect use of winc command");
                    } else {
                        LOG("winc = %llu", tmp);

                        if (!sc.pos.flipped)
                           sc.increment = tmp;
                    }

                } else if (!std::strcmp(s, "binc")) {

                    if (!read_next_ulong(tmp)) {
                        LOG("Incorrect use of binc command");
                    } else {
                        LOG("binc = %llu", tmp);

                        if (sc.pos.flipped)
                           sc.increment = tmp;
                    }

                } else if (!std::strcmp(s, "movestogo")) {

                    if (!read_next_ulong(tmp)) {
                        LOG("Incorrect use of movestogo command");
                    } else {
                        LOG("movestogo = %llu", tmp);
                        sc.moves_per_session = (std::uint32_t)tmp;
                    }

                } else if (!std::strcmp(s, "depth")) {

                    if (!read_next_ulong(tmp)) {
                        LOG("Incorrect use of depth command");
                    } else {
                        LOG("depth = %llu", tmp);
                        sc.max_depth = (std::uint32_t)tmp;
                    }

                } else if (!std::strcmp(s, "nodes")) {

                    if (!read_next_ulong(tmp)) {
                        LOG("Incorrect use of nodes command");
                    } else {
                        LOG("nodes = %llu", tmp);
                        /* = tmp;*/
                    }

                } else if (!std::strcmp(s, "mate")) {

                    if (!read_next_ulong(tmp)) {
                        LOG("Incorrect use of mate command");
                    } else {
                        LOG("mate = %llu", tmp);
                        /* = tmp;*/
                    }

                } else if (!std::strcmp(s, "movetime")) {

                    if (!read_next_ulong(tmp)) {
                        LOG("Incorrect use of movetime command");
                    } else {
                        LOG("movetime = %llu", tmp);
                        sc.movetime = tmp;
                    }

                } else if (!std::strcmp(s, "infinite")) {
                    LOG("infinite command");
                } else {
                    LOG("Unrecognized token : \"%s\"", s);
                }

                i = 0;
            }

            if (c == '\n') {
                break;
            }

        } else {

            if (i >= (MAX_UCICMD_LEN - 1)) {
                parse_searchmoves = false;
                i = 0;
                s[MAX_UCICMD_LEN - 1] = 0;
                LOG("Token \"%s...\" exceeds max token length", s);

                flush_up_to_whitespace();

                if (c == '\n')
                    break;
                else
                    continue;
            }

            s[i++] = (char)c;

        }
    }

    std::thread search(search_thread, &sc);
    search.detach();
}

void handle_position_fen()
{
    static const std::size_t MAX_UCICMD_LEN = 128;

    int current_field = 0;
    bool new_field = false;
    std::size_t i = 0;
    char s[MAX_UCICMD_LEN];

    while (current_field < 5) {

        c = std::fgetc(stdin);

        if (c == EOF) {

            handle_eof();

        } else if (std::isspace(c)) {

            if (c == '\n') {
                LOG("Invalid use of position command");
                return;
            }

            new_field = true;

        } else {

            if (i >= (MAX_UCICMD_LEN - 1)) {
                s[MAX_UCICMD_LEN - 1] = 0;

                LOG("Token \"%s...\" exceeds max token length", s);
                return;
            }

            if (new_field) {
                s[i++] = ' ';
                new_field = false;
                current_field++;
            }

            s[i++] = (char)c;

        }
    }

    parse_fen_to_position(s, sc.pos);

    handle_position_moves();
}

void handle_position_moves()
{
    static const std::size_t MAX_UCICMD_LEN = 9;

    std::size_t i = 0;
    char s[MAX_UCICMD_LEN];
    Move move;

    if (c == '\n') {
        return;
    }

    //initially skip all the characters up to the first
    //character on the first LAN string. e.g.
    //"position startpos moves e2e4 c7c5 g1f3 d7d6 d2d4 c5d4 f3d4 b8c6"
    //                         ^up to this character
    while (1) {

        c = std::fgetc(stdin);

        if (c == EOF) {

            handle_eof();

        } else if (std::isspace(c)) {

            if (c == '\n') {
                return;
            }

            if (i) {

                s[i] = 0;

                if (!std::strcmp(s, "moves")) {
                    break;
                } else {
                    LOG("Unrecognized token : \"%s\"", s);
                    return;
                }

            }

        } else {

            if (i >= (MAX_UCICMD_LEN - 1)) {
                s[MAX_UCICMD_LEN - 1] = 0;

                LOG("Token \"%s...\" exceeds max token length", s);
                return;
            }

            s[i++] = (char)c;

        }
    }

    if (!flush_whitespace()) {
        LOG("Invalid use of position command");
        return;
    }

    i = 0;

    while (1) {

        if (c == EOF) {

            handle_eof();

        } else if (std::isspace(c)) {

            if (i) {

                if (!lan_to_move(sc.pos, s, move)) {
                    LOG("Unrecognized token : \"%s\"", s);
                    return;
                }

                if (sc.pos.flipped)
                    move = flip_move(move);

                make_move(sc.pos, move);

                /*
                printf("Positions of our pieces:\n");
                PRINT_BITBOARD(sc.pos.colours[US]);

                printf("Positions of their pieces:\n");
                PRINT_BITBOARD(sc.pos.colours[THEM]);
                */

                s[i] = 0;
                i = 0;

            }

            if (c == '\n') {
                break;
            }

        } else {

            if (i >= 5) {
                s[5] = 0;

                LOG("Token \"%s...\" exceeds max token length", s);
                return;
            }

            s[i++] = (char)c;
            s[i] = '\0';

        }

        c = std::fgetc(stdin);
    }
}

void handle_position()
{
    static const std::size_t MAX_UCICMD_LEN = 9;

    std::size_t i = 0;
    char s[MAX_UCICMD_LEN];

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
                    parse_fen_to_position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", sc.pos);
                    handle_position_moves();
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

            if (i >= (MAX_UCICMD_LEN - 1)) {
                flush_up_to_char('\n');

                s[MAX_UCICMD_LEN - 1] = 0;

                LOG("Token \"%s...\" exceeds max token length", s);
                break;
            }

            s[i++] = (char)c;

        }
    }
}

/*void handle_register()
{
    flush_up_to_char('\n');
}*/

void handle_setoption()
{
    flush_up_to_char('\n');
}

void handle_isready()
{
    send_cmd("readyok");
}

void handle_ucinewgame()
{
    //not necessary
    //std::memset((void*)&sc.pos, 0, sizeof(((SearchController*)0)->pos));
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

//separate handler for single word commands that aren't
//bound to be followed by other words (e.g "isready\n", "    \t\t  ucinewgame\n")
void handle_simple_commands(char *cmd)
{
    if (!std::strcmp(cmd, "isready")) {
        LOG("isready command");
        handle_isready();
    } else if (!std::strcmp(cmd, "stop")) {
        LOG("stop command");
        handle_stop();
    } else if (!std::strcmp(cmd, "ponderhit")) {
        LOG("ponderhit command");
        handle_ponderhit();
    } else if (!std::strcmp(cmd, "quit")) {
        LOG("quit command");
        handle_quit();
    } else if (!std::strcmp(cmd, "ucinewgame")) {
        LOG("ucinewgame command");
        handle_ucinewgame();
    } else if (!std::strcmp(cmd, "print")) {
        print_position(sc.pos);
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
    } /*else if (opt.registration && !std::strcmp(cmd, "register")) {
        LOG("register command");
        handle_register();
    }*/ else if (!std::strcmp(cmd, "setoption")) {
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

    send_cmd("id name Monochrome");
    send_cmd("id author mkchan ZirconiumX Gikoskos");

    //send options to the GUI here
    //send_cmd("option \n");

    send_cmd("uciok");
    parse_fen_to_position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", sc.pos);

    static const std::size_t MAX_UCICMD_LEN = 11;
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
            if (i >= (MAX_UCICMD_LEN - 1)) {

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
