/*
MIT License

Copyright (c) 2017 CPirc
Copyright (c) 2018 CPirc
Copyright (c) 2019 CPirc

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

#ifndef TT_H
#define TT_H

#include "types.h"
#include "misc.h"

// TTEntry.data layout
//     Search: Depth (7b) - Eval (32b) - Flag (2b) - Move (18b)
//     Perft:  Depth (7b) - Nodes (57b)

#define TT_DEPTH_SHIFT (0)
#define TT_EVAL_SHIFT  (7)
#define TT_FLAG_SHIFT  (39)
#define TT_MOVE_SHIFT  (41)
#define TT_NODES_SHIFT (7)

#define TT_EVAL_MASK  (0xFFFFFFFF)
#define TT_FLAG_MASK  (0x3)
#define TT_DEPTH_MASK (0x7F)
#define TT_MOVE_MASK  (0x3FFFF)
#define TT_NODES_MASK (0x1FFFFFFFFFFFFFF)

enum flag : int {
    TT_LOWER=0,
    TT_UPPER,
    TT_EXACT
};

/* A transposition table entry */
struct TTEntry {
    std::uint64_t data;
    std::uint64_t hash_key;
};

/* The transposition table */
struct TT {
    int size;
    TTEntry* data;
};

extern bool tt_create(TT* tt, const std::uint32_t megabytes);
extern TTEntry tt_poll(TT* tt, const std::uint64_t key);
extern bool tt_clear(TT* tt);
extern bool tt_free(TT* tt);
extern bool tt_add(TT* tt, const std::uint64_t hash_key, const int move, const int depth, const int flag, const int eval);
extern bool tt_add_perft(TT* tt, const std::uint64_t hash_key, const int depth, const uint64_t nodes);

inline int eval_to_tt(const int eval, const int ply)
{
    assert(ply >= 0);

    if (eval >= INF - MAX_PLY) {
        return eval + ply;
    }
    if (eval <= -INF + MAX_PLY) {
        return eval - ply;
    }
    return eval;
}

inline int eval_from_tt(const int eval, const int ply)
{
    assert(ply >= 0);

    if (eval >= INF - MAX_PLY) {
        return eval - ply;
    }
    if (eval <= -INF + MAX_PLY) {
        return eval + ply;
    }
    return eval;
}

/* Get the transposition table entry depth */
inline int tt_depth(const std::uint64_t data)
{
    int depth = (data >> TT_DEPTH_SHIFT) & TT_DEPTH_MASK;
    assert(depth >= 0);
    assert(depth < MAX_PLY);
    return depth;

    //return (data >> TT_DEPTH_SHIFT) & TT_DEPTH_MASK;
}

/* Get the transposition table entry eval */
inline int tt_eval(const std::uint64_t data)
{
    int eval = (data >> TT_EVAL_SHIFT) & TT_EVAL_MASK;
    assert(eval <= INF + MAX_PLY);
    assert(eval >= -INF - MAX_PLY);
    //assert(eval <= INF);
    //assert(eval >= -INF);
    return eval;

    //return (data >> TT_EVAL_SHIFT) & TT_EVAL_MASK;
}

/* Get the transposition table entry flag */
inline int tt_flag(const std::uint64_t data)
{
    int flag = (data >> TT_FLAG_SHIFT) & TT_FLAG_MASK;
    assert(flag == TT_EXACT || flag == TT_LOWER || flag == TT_UPPER);
    return flag;

    //return (data >> TT_FLAG_SHIFT) & TT_FLAG_MASK;
}

/* Get the transposition table entry move */
inline int tt_move(const std::uint64_t data)
{
    return (data >> TT_MOVE_SHIFT) & TT_MOVE_MASK;
    //return (Move)((data >> TT_MOVE_SHIFT) & TT_MOVE_MASK);
}

/* Get the transposition table entry node count for TT perft */
inline int tt_nodes(const std::uint64_t data)
{
    return (data >> TT_NODES_SHIFT) & TT_NODES_MASK;
}

#endif
