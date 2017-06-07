#include "tt.h"
#include <cstring>
#include <cassert>

bool tt_create(TT* tt, const int megabytes)
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

    // FIX ME
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

    int index = hash_key%tt->size;

    TTEntry entry;
    entry.data = 0ULL;
    entry.hash_key = hash_key;

    entry.data = ((std::uint64_t)(move & TT_MOVE_MASK) << TT_MOVE_SHIFT) |
                 ((std::uint64_t)(depth & TT_DEPTH_MASK) << TT_DEPTH_SHIFT) |
                 ((std::uint64_t)(flag & TT_FLAG_MASK) << TT_FLAG_SHIFT) |
                 ((std::uint64_t)(eval & TT_EVAL_MASK) << TT_EVAL_SHIFT);

    tt->data[index] = entry;

    /*
    */
    // Test
    TTEntry get_entry = tt_poll(tt, hash_key);

    int get_move = tt_move(get_entry.data);
    int get_depth = tt_depth(get_entry.data);
    int get_flag = tt_flag(get_entry.data);
    int get_eval = tt_eval(get_entry.data);

    assert(get_entry.hash_key == entry.hash_key);
    assert(get_entry.data == entry.data);

    assert(get_move == move);
    assert(get_depth == depth);
    assert(get_flag == flag);
    assert(get_eval == eval);

    return true;
}
