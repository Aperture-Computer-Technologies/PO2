//
// Created by MassiveAtoms on 1/31/21.
//

#ifndef PO2_CHAINING_SIMD_H
#define PO2_CHAINING_SIMD_H

#include <bits/stdint-intn.h>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>

#include "immintrin.h"  // fo
#include "nmmintrin.h"  // for SSE4.2

using std::cout;
using std::vector;

namespace helper {
    vector<int> prime_sizes
        = {127,    251,    479,     911,     1733,    3299,    6269,     11923,    22669,    43093,    81883,   155579,
           295601, 561667, 1067179, 2027659, 3852553, 7319857, 13907737, 26424707, 50206957, 95393219, 18124717};
}

size_t next_prime(int n)
{
    for (const int x : helper::prime_sizes) {
        if (x > n) {
            return n;
        }
    }
}

class SIMD {
  public:
    SIMD();
    ~SIMD(){};
    void reserve(int size);
    void insert(std::initializer_list<int> init_list);
    int& operator[](const int& k);
    int& operator[](int&& k);
    void erase(int key);

    bool contains(const int& key) const;
    void makeEmpty();

    int32_t DELETED_STATE = -1;  // set hashcode to this if deleted
  private:
    int32_t hasher(const int& x) const;
    void hasher_state_gen();
    vector<size_t> hash_state;
    vector<std::pair<int, int>> data_array;
};

/*
 * this changes the state for rehashes and init.
 *
 */
void SIMD::hasher_state_gen()
{
    std::vector<size_t> state(127);
    std::generate(state.begin(), state.end(), gen_int);
    hash_state = state;
}

// calcs hash, modulo it later
int SIMD::hasher(const int& key) const
{
    static std::hash<int> hf;
    size_t hash = hf(key);
    int32_t final_hash;
    size_t index;
    for (int i = 0; i < sizeof(hash); i++) {
        index += hash & 0x00000000000000ff;
        hash = hash >> 8;
        final_hash = final_hash ^ hash_state[index + i];
    }
    return final_hash;
}

// _mm_cmpeq_epi64,
//
#endif  // PO2_CHAINING_SIMD_H
