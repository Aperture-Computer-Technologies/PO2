#ifndef Cuckoo_H
#define Cuckoo_H

#include <algorithm>
#include <deque>
#include <numeric>
#include <tuple>
#include <vector>

#include "fastmod.h"
#include "helpers.h"
using std::vector;

/*
 * int32_t pos;
 * bool exists;
 * int hasher;
 */
struct Result {
    /*
     * Result( int position, bool isthere, int which hashfunction)
     */
    Result(int32_t position, bool is_there, int32_t h1, int32_t h2)
        : pos{position}, exists{is_there}, hash1{h1}, hash2{h2} {};
    int32_t pos;
    bool exists;
    int32_t hash1;
    int32_t hash2;
};

/*
Cuckoo, but with a modulo trick
 https://github.com/lemire/fastmod
 *
 */

template <typename K, typename V>
class Cuckoo {
  public:
    Cuckoo();
    explicit Cuckoo(int size);
    ~Cuckoo() { clear(); };
    void insert(const std::pair<K, V> kv);
    bool contains(const K& key) const;
    V& operator[](const K& k);
    void clear();
    int32_t bucket_count() { return store.size(); };
    int32_t size() { return inserted_n; };
    void reserve(int size);
    void erase(const K& key);
    void rehash();
    //    V& operator[](V&& k);

  private:
    int32_t DELETED = -1;
    int32_t EMPTY = -2;
    struct Element {
        Element(K key_, V* val_, int32_t hash_1, int32_t hash_2)
            : hash1{hash_1}, hash2{hash_2}, key{key_}, val{val_} {};
        Element() : hash1{-2}, hash2{-2}, key{0}, val{0} {};  // fix this, or no magic. is empty
        Element(const Element& e) : hash1{e.hash1}, hash2{e.hash2}, key{e.key}, val{e.val} {};
        int32_t hash1;
        int32_t hash2;
        K key;
        V* val;
    };
    int inserted_n;
    uint64_t modulo_help;
    vector<int32_t> hash_state1;
    vector<int32_t> hash_state2;
    float lf_max;
    vector<Element> store;
    std::deque<V> valuestore;
    vector<V*> open_slots;

    int32_t hasher1(const K& key) const;
    int32_t hasher2(const K& key) const;
    void hasher_state_gen();
    Result prober(const K& key, const int32_t& hash1, const int32_t& hash2) const;
    void rehash(int size);
    Result contains_key(const K& key) const;
};

/*
 * constructor calls constructor with explicit size.
 * reason why i'm not doing Cuckoo(size=something) is compiler complains
 */
template <typename K, typename V>
Cuckoo<K, V>::Cuckoo() : Cuckoo{Cuckoo<K, V>(251)}
{
}
template <typename K, typename V>
Cuckoo<K, V>::Cuckoo(int size)
    : store{vector<Element>(2 * size)},
      inserted_n{0},
      modulo_help(fastmod::computeM_s32(2 * size)),
      lf_max{0.5},
      valuestore{},
      open_slots{}
{
    hasher_state_gen();
}
/*
 * function to generate hashes.
 * it's tabulation hashing.
 */
template <typename K, typename V>
int32_t Cuckoo<K, V>::hasher1(const K& key) const
{
    static std::hash<K> hf;
    int32_t hash = hf(key);
    int32_t final_hash = 0;
    int32_t pos = 0;
    for (int i = 0; i < sizeof(hash); i++) {
        pos = hash & 0x00000000000000ff;
        hash = hash >> 8;
        final_hash = final_hash ^ hash_state1[pos + i];
    }
    if (final_hash < 0) {
        final_hash = final_hash * -1;
    }
    return final_hash;
}

/*
 * function to generate hashes.
 * it's tabulation hashing.
 */
template <typename K, typename V>
int32_t Cuckoo<K, V>::hasher2(const K& key) const
{
    static std::hash<K> hf;
    int32_t hash = hf(key);
    int32_t final_hash = 0;
    int32_t pos = 0;
    for (int i = 0; i < sizeof(hash); i++) {
        pos = hash & 0x00000000000000ff;
        hash = hash >> 8;
        final_hash = final_hash ^ hash_state2[pos + i];
    }
    final_hash += hasher1(hash);
    if (final_hash < 0) {
        final_hash = final_hash * -1;
    }
    return final_hash;
}

/*
 * generate random values so hasher can use them
 */
template <typename K, typename V>
void Cuckoo<K, V>::hasher_state_gen()
{
    std::vector<int32_t> state(259);
    std::generate(state.begin(), state.end(), gen_integer);
    hash_state1 = state;
    std::generate(state.begin(), state.end(), gen_integer);
    hash_state2 = state;
}

/*
 * probing function.
 * it should probe locations, and stop when:
 * 1. hash = empty
 * 2. same key
 */
template <typename K, typename V>
Result Cuckoo<K, V>::prober(const K& key, const int32_t& hash1, const int32_t& hash2) const
{
    unsigned long size = store.size();
    int32_t pos1 = fastmod::fastmod_s32(hash1, modulo_help, size);
    int32_t pos2 = fastmod::fastmod_s32(hash2, modulo_help, size);
    if (store[pos1].hash1 == hash1 && store[pos1].key == key) {
        return Result{pos1, true, hash1, hash2};
    }
    else if (store[pos2].hash2 == hash2 && store[pos1].key == key) {
        return Result{pos2, true, hash1, hash2};
    }
    else {
        return Result{pos1, false, hash1, hash2};
    }
}
/*
 * returns bool, index, hash1, hash2
 * probe bucket arr, and if the resulting index is empty, key doesn't exist
 */
template <typename K, typename V>
Result Cuckoo<K, V>::contains_key(const K& key) const
{
    int32_t hash1 = hasher1(key);
    int32_t hash2 = hasher2(key);
    return prober(key, hash1, hash2);
}

/*
 * std::unordered has this. i should probably just return contains_key()[0]
 */
template <typename K, typename V>
bool Cuckoo<K, V>::contains(const K& key) const
{
    return contains_key(key).contains;
}

/*
 * inserts element, returns nothing.
 * detects if insertion will result in >max load factor, rehashes if it will
 * then check for existence. if there is, stop.
 * else, insert.
 *
 */
template <typename K, typename V>
void Cuckoo<K, V>::insert(const std::pair<K, V> kv)
{
    if (((inserted_n + 1) / (float)store.size()) > lf_max) {
        rehash();
    }

    auto pos_info = contains_key(kv.first);
    if (pos_info.exists) {
        return;
    }
    V* val_ptr = nullptr;
    if (open_slots.size()) {
        val_ptr = open_slots.back();
        open_slots.pop_back();
        *val_ptr = kv.second;
    }
    else {
        valuestore.emplace_back(kv.second);
        val_ptr = &valuestore.back();
    }
    Element current{kv.first, val_ptr, pos_info.hash1, pos_info.hash2};
    auto pos = pos_info.pos;
    auto size = store.size();
    for (int i = 0; i < 20; i++) {
        std::swap(store[pos], current);
        if (current.hash1 < 0) {
            break;
        }
        pos = fastmod::fastmod_s32(current.hash2, modulo_help, size);
        std::swap(store[pos], current);
        if (current.hash1 < 0) {
            break;
        }
        pos = fastmod::fastmod_s32(current.hash1, modulo_help, size);
    }
    inserted_n++;
}
/*check for existence.
 * if there is, return value
 * if there isn't, insert V{}
 *
 */
template <typename K, typename V>
V& Cuckoo<K, V>::operator[](const K& k)
{
    auto pos_info = contains_key(k);
    if (pos_info.exists) {
        return *store[pos_info.pos].val;
    }

    V* val_ptr = nullptr;
    if (open_slots.size()) {
        val_ptr = open_slots.back();
        open_slots.pop_back();
        *val_ptr = V{};
    }
    else {
        valuestore.emplace_back(V{});
        val_ptr = &valuestore.back();
    }
    Element current{k, val_ptr, pos_info.hash1, pos_info.hash2};
    auto pos = pos_info.pos;
    auto size = store.size();
    for (int i = 0; i < 20; i++) {
        std::swap(store[pos], current);
        if (current.hash1 < 0) {
            break;
        }
        pos = fastmod::fastmod_s32(current.hash2, modulo_help, size);
        std::swap(store[pos], current);
        if (current.hash1 < 0) {
            break;
        }
        pos = fastmod::fastmod_s32(current.hash1, modulo_help, size);
    }
    inserted_n++;
    return *store[pos].val;
}
/*
 * just delete everything
 */
template <typename K, typename V>
void Cuckoo<K, V>::clear()
{
    open_slots.clear();
    valuestore.clear();
    store.clear();
    inserted_n = 0;
}
template <typename K, typename V>
void Cuckoo<K, V>::rehash(int size)
{
    vector<Element> arr_new(size);
    uint64_t helper = fastmod::computeM_s32(size);
    for (auto& x : store) {
        if (x.hash1 < 0) {
            continue;
        }

        int32_t pos = fastmod::fastmod_s32(x.hash1, helper, size);
        auto current = x;
        for (int i = 0; i < 20; i++) {
            std::swap(arr_new[pos], current);
            if (current.hash1 < 0) {
                break;
            }
            pos = fastmod::fastmod_s32(current.hash2, modulo_help, size);
            std::swap(arr_new[pos], x);
            if (current.hash1 < 0) {
                break;
            }
            pos = fastmod::fastmod_s32(x.hash1, helper, size);
        }
    }
    store = arr_new;
    modulo_help = helper;
}
/*
 * increase size and rehash
 */
template <typename K, typename V>
void Cuckoo<K, V>::rehash()
{
    int size = helper::next_prime(store.size());
    rehash(size);
}

template <typename K, typename V>
void Cuckoo<K, V>::reserve(int size)
{
    int s = 1 + (size / lf_max);
    if (s < store.size()) {
        return;
    }
    rehash(s);
}

template <typename K, typename V>
void Cuckoo<K, V>::erase(const K& key)
{
    auto pos_info = contains_key(key);
    if (not pos_info.exists) {
        return;
    }
    auto pos = pos_info.pos;
    store[pos].hash1 = DELETED;
    store[pos].hash2 = DELETED;
    open_slots.emplace_back(store[pos].val);
    *store[pos].val = V{};
    store[pos].val = nullptr;
    store[pos].key = K{};
    inserted_n--;
}

#endif
