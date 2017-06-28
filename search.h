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

#ifndef SEARCH_H
#define SEARCH_H

#include "position.h"
#include "move.h"
#include "tt.h"
#include <ctime> // clock_t, clock()

/* Records search statistics */
struct Stats {
    std::uint64_t node_count;
#ifdef TESTING
    std::uint64_t fail_highs;
    std::uint64_t first_move_fail_highs;
#endif
};

/* A data structure to pass local parameters thru */
struct SearchStack {
    std::uint8_t ply;
    Move ml[256];
    int score[256];
    Move killers[2];
    Stats* stats;
};

struct SearchController {
    Position pos;
    std::uint32_t max_depth;
    std::uint32_t moves_per_session;
    clock_t increment;
    clock_t search_start_time;
    clock_t search_end_time;
    clock_t our_clock;
    clock_t movetime;
    TT tt;
};

extern void search_thread(void* params);
extern void clear_ss(SearchStack* ss, int size);
extern Move next_move(SearchStack* ss, int& size);

#endif
