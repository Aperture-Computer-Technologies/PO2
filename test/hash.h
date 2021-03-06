
#ifndef HASH_H
#define HASH_H

#include <cstddef>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <vector>

#include "./../tools/random.h"

size_t example(int key, int size) { return key % size; }

size_t dumb_bitmask(int key, int size)
{
    int ans = key >> 21;
    if (ans >= size) {
        return ans >> 1;
    }
    else {
        return ans;
    }
}  // Use folding on a string, summed 4 bytes at a time
size_t sfold(int key, int size)
{
    size_t x = key * 256;
    if (x < 0) {
        x = x * -1;
    }
    x = x % size;
    return x;
}

std::vector<size_t> hasher_state_gen()
{
    std::vector<size_t> state(127);
    std::generate(state.begin(), state.end(), gen_int);
    return state;
}

// Potentially doesn't work
std::vector<size_t> hasher_state = hasher_state_gen();
template <typename T>
size_t tabular_hashing(T key, size_t size)
{
    std::vector<int_fast8_t> segments = reinterpret_cast<std::vector<int_fast8_t>>(key);
    size_t res{};
    for (const auto& x : segments) {
        res ^= hasher_state[x];
    }
}

#endif