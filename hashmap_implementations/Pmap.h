#ifndef PO2_PMAP_H
#define PO2_PMAP_H

//
// Created by MassiveAtoms on 1/31/21.
//

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
//template <class K>
//template <class V>
class Pmap {
  public:
    explicit Pmap(int size = 101);
    ~Pmap(){};
    void reserve(int size);                             // set size of the array
    void insert(std::pair<int,int> init_list);  // insert
    int& operator[](const int& k);                      // lookup and if you can, insert
    int& operator[](int&& k);                           // lookup and if you can, insert
    void erase(int key);
    int total_keys = 0;
  private:
    size_t buckets;
    enum class state {EMPTY, DELETED, ACTIVE};
//    int8_t EMPTY = 0;
//    int8_t DELETED = 1;
//    int8_t ACTIVE = 2;
    struct HashEntry
    {
        int value;
        size_t hash;
        state status;
        HashEntry(): status{state::EMPTY}{};
        HashEntry(size_t hash_, int val): status{state::ACTIVE}, hash{hash_}, value{val}{};
    };
    size_t hasher(const int& x) const;
    void hasher_state_gen();
    vector<size_t> hash_state;
    vector<HashEntry> array;
    size_t prober(size_t hash);
    void rehash(int size);
};

size_t Pmap::prober(size_t hash) {
    size_t loc = hash % buckets;
    while (array[loc].status!=state::EMPTY && array[loc].hash != hash){
        loc++;
        if (loc > array.size()){
            loc -= array.size();
        }
    }
}

void Pmap::insert(std::pair<int,int> init_list) {
    if ((total_keys + 1)/buckets > 0.5){// change 0.5 to some var
        rehash(helper::next_prime(total_keys));

    }
    size_t h = hasher(init_list.first);
    size_t loc = prober(h);
    array[loc] = HashEntry(h, init_list.second);
    total_keys++;
}

int & Pmap::operator[](const int& k) {
    auto h = hasher(k);
    auto loc = prober(h);
    if (array[loc].hash == h){
        return array[loc].value;
    }
    else{
        array[loc] = HashEntry{h, 0};
        return array[loc].value;
    }
}

int & Pmap::operator[](int&& k) {
    auto h = hasher(k);
    auto loc = prober(h);
    if (array[loc].hash == h){
        return array[loc].value;
    }
    else{
        array[loc] = HashEntry{h, 0};
        return array[loc].value;
    }
}

void Pmap::reserve(int size) {
    rehash(size);
}

Pmap::Pmap(int size): array{vector<HashEntry>(size)} {
    hasher_state_gen();
    buckets = size;
}

void Pmap::rehash(int size) {
    vector<HashEntry> new_arr(size);
    for (const HashEntry& x: array){
        if (x.status != state::ACTIVE){
            continue;
        }
        size_t loc = x.hash % size;
        while (new_arr[loc].status!=state::EMPTY && new_arr[loc].hash != x.hash){
            loc++;
            if (loc > new_arr.size()){
                loc -= new_arr.size();
            }
        }
        new_arr[loc] = x;

    }
    buckets = size;
    array = new_arr;
}

void Pmap::erase(int key) {
    auto loc = prober(hasher(key));
    array[loc].status = state::DELETED;
    total_keys--;
}







/*
 * this changes the state for rehashes and init.
 *
 */
void Pmap::hasher_state_gen()
{
    std::vector<size_t> state(127);
    std::generate(state.begin(), state.end(), gen_int);
    hash_state = state;
}

// calcs hash, modulo it later
// tabulation hash
size_t Pmap::hasher(const int& key) const
{
    static std::hash<int> hf;
    size_t hash = hf(key);
    size_t final_hash;
    size_t index;
    for (int i = 0; i < sizeof(hash); i++) {
        index += hash & 0x00000000000000ff;
        hash = hash >> 8;
        final_hash = final_hash ^ hash_state[index + i];
    }
    return final_hash;
}


#endif  // PO2_PMAP_H
