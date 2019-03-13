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

#ifndef MISC_H
#define MISC_H

#include <random>

#include "types.h"

#define MAX_PLY (64)
#define INF (30000)

/* The Mersenne twister random number generator */
static std::mt19937 rng;

/* Seed the random number generator with the specified seed */
inline void seed_rng(std::uint32_t seed) { rng.seed(seed); }

/* Get a random 64 bit number */
inline std::uint64_t get_rand64() {
    static std::uniform_int_distribution<std::uint64_t> dist;
    std::uint64_t r;
    do {
        r = dist(rng);
    } while (!r);
    return r;
}

#endif
