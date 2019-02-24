/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Dan Ravensloft <dan.ravensloft@gmail.com>
 * Copyright (c) 2017 CPirc
 * Copyright (c) 2018 CPirc
 * Copyright (c) 2019 CPirc
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "tt.h"
#include <cstring>
#include <cassert>

bool tt_create(TT* tt, const std::uint32_t megabytes)
{
    assert(tt);

    if (megabytes <= 0) {
        return false;
    } else if (megabytes > 512) {
        return false;
    }

    int num_entries = 1024*1024*megabytes/sizeof(TTEntry);

    assert(num_entries*sizeof(TTEntry) <= 1024*1024*megabytes);

    tt->data = (TTEntry *)malloc(num_entries*sizeof(TTEntry));

    if (tt->data) {
        tt->size = num_entries;
        return true;
    }

    tt->size = 0;
    return false;
}

TTEntry tt_poll(TT* tt, const std::uint64_t key)
{
    assert(tt);
    assert(tt->size);
    assert(tt->data);

    int index = key%tt->size;

    assert(index < tt->size);

    return tt->data[index];
}

bool tt_clear(TT* tt)
{
    assert(tt);

    if (!tt->data) {
        return false;
    }

    memset(tt->data, 0, tt->size*sizeof(TTEntry));
    return true;
}

bool tt_free(TT* tt)
{
    assert(tt);

    if (!tt->data) {
        return false;
    }

    tt->size = 0;
    free(tt->data);
    return true;
}

bool tt_add(TT* tt, const std::uint64_t hash_key, const int move, const int depth, const int flag, const int eval)
{
    assert(tt);
    assert(move != 0);
    assert(depth >= 0);
    assert(flag == TT_EXACT || flag == TT_LOWER || flag == TT_UPPER);

    if (!tt->data) {
        return false;
    }

    int index = hash_key % tt->size;

    TTEntry entry;
    entry.data = 0ULL;
    entry.hash_key = hash_key;

    entry.data = ((std::uint64_t)(move & TT_MOVE_MASK) << TT_MOVE_SHIFT) |
                 ((std::uint64_t)(depth & TT_DEPTH_MASK) << TT_DEPTH_SHIFT) |
                 ((std::uint64_t)(flag & TT_FLAG_MASK) << TT_FLAG_SHIFT) |
                 ((std::uint64_t)(eval & TT_EVAL_MASK) << TT_EVAL_SHIFT);

    tt->data[index] = entry;

    return true;
}

bool tt_add_perft(TT* tt, const std::uint64_t hash_key, const int depth, const uint64_t nodes)
{
    assert(tt);
    assert(depth != 0);

    if (!tt->data) {
        return false;
    }

    int index = hash_key % tt->size;

    TTEntry entry;
    entry.data = 0ULL;
    entry.hash_key = hash_key;

    entry.data = ((std::uint64_t)(depth & TT_DEPTH_MASK) << TT_DEPTH_SHIFT) |
                 ((std::uint64_t)(nodes & TT_NODES_MASK) << TT_NODES_SHIFT);

    tt->data[index] = entry;

    return true;
}
