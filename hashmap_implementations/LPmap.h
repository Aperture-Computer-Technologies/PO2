//
// Created by MassiveAtoms on 1/31/21.
//

#ifndef PO2_CHAINING_SIMD_H
#define PO2_CHAINING_SIMD_H

#include <bits/stdint-intn.h>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <list>
#include <vector>

#include "../tools/random.h"

using std::cout;
using std::vector;

namespace helper {
    vector<int> prime_sizes
        = {127,    251,    479,     911,     1733,    3299,    6269,     11923,    22669,    43093,    81883,   155579,
           295601, 561667, 1067179, 2027659, 3852553, 7319857, 13907737, 26424707, 50206957, 95393219, 18124717};

    size_t next_prime(const int& n)
    {
        for (const int x : prime_sizes) {
            if (x > n) {
                size_t t = x;
                return t;
            }
        }
    }
}  // namespace helper

class LPmap {
  public:
    explicit LPmap(int size = 101);
    ~LPmap(){};
    void reserve(int size);                             // set size of the array
    void insert(std::initializer_list<int> init_list);  // insert
    int& operator[](const int& k);                      // lookup and if you can, insert
    int& operator[](int&& k);                           // lookup and if you can, insert
    void erase(int key);
    int size;
    double max_load = 0.5;

    bool contains(const int& key) const;
    void makeEmpty();

  private:
    int32_t DELETED_STATE = -1;  // set hashcode to this if deleted
    int32_t EMPTY_STATE = 0;
    int32_t myHash(const int& x) const;
    void hasher_state_gen();
    int32_t calc_pos(const int& key) const;
    int32_t prober(const int& key) const;
    void rehash(int size);
    void rehash();
    vector<size_t> hash_state;
    vector<std::pair<int, int>> array;  // hash, value
    int currentSize;
};

// constructor
LPmap::LPmap(int size) : array(size) {
    makeEmpty();
    hasher_state_gen();
}

// "deletes" all elements (lazy)
void LPmap::makeEmpty()
{
    for (auto& x : array) {
        x.first = EMPTY_STATE;
    }
}

// dont know if i need it
// but calculates the position it should be inserted in without probing
int32_t LPmap::calc_pos(const int& key) const { return myHash(key) % currentSize; }

// probes where the element should be located at, with probing
int32_t LPmap::prober(const int& key) const
{
    int hash = myHash(key);
    int position = hash % currentSize;
    int count = 0;
    while (array[position].first != EMPTY_STATE && array[position].first != hash) {
        position++;
        count++;
        if (position >= array.size()) {
            position -= array.size();
        }
    }
    return position;
}

// check if key exists (technically, if object with same hash exists in correct location)
bool LPmap::contains(const int& key) const
{
    auto pos = prober(key);
    return array[pos].first == myHash(key);
}

// insert
void LPmap::insert(std::initializer_list<int> init_list)
{
    if (max_load < (size + 1) / currentSize) {
        rehash();
    }
    vector<int> access = {init_list}; // key, value = 0, 1
    if (!contains(access[0])) {
        auto pos = prober(access[0]);
        if (pos - calc_pos(access[0]) > 10){
            rehash();
            pos = prober(access[0]);
        }
        array[pos] = std::pair<int, int>{myHash(access[0]), access[1]};
    }
}

// lookup, insert on nonexistence
int& LPmap::operator[](const int& k)
{
    if (contains(k)) {
        return array[prober(k)].second;
    }
    else {
        insert({k, 0});
        return array[prober(k)].second;
    }
}
// second one
int& LPmap::operator[](int&& k)
{
    if (contains(k)) {
        return array[prober(k)].second;
    }
    else {
        insert({k, 0});
        return array[prober(k)].second;
    }
}

// erase
void LPmap::erase(int key)
{
    auto loc = prober(key);
    array[loc].first = DELETED_STATE;
}

void LPmap::rehash(int size)
{
    vector<std::pair<int, int>> new_arr(size);
    for (const auto& x : array) {
        if (x.first == DELETED_STATE) {
            continue;
        }
        int loc = x.first % size;
        int value = x.second;
        while (new_arr[loc].first != EMPTY_STATE) {
            loc++;
            if (loc >= size) {
                loc -= size;
            }
        }
        new_arr[loc] = {x.first, x.second};
    }
    for (auto& x: new_arr){
        if (!x.first){
            x.first = EMPTY_STATE;
        }
    }
    array = new_arr;
    currentSize = size;
}

void LPmap::rehash()
{
    int size = helper::next_prime(currentSize);
    rehash(size);
}

void LPmap::reserve(int size) { rehash(size); }

/*
 * this changes the state for rehashes and init.
 *
 */
void LPmap::hasher_state_gen()
{
    std::vector<size_t> state(127);
    std::generate(state.begin(), state.end(), gen_int);
    hash_state = state;
}

// calcs hash, modulo it later
// tabulation hash
int LPmap::myHash(const int& key) const
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
