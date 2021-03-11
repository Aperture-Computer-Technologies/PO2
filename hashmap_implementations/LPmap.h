#ifndef LP_H
#define LP_H

#include <algorithm>
#include <deque>
#include <numeric>
#include <tuple>
#include <vector>

#include "helpers.h"
#include "fastmod.h"
using std::vector;

/*
LP, but with a modulo trick
 https://github.com/lemire/fastmod
 *
 */

template <typename K, typename V>
class LP {
  public:
    LP();
    explicit LP(int size);
    ~LP() { clear(); };
    void insert(const std::pair<K, V> kv);
    bool contains(const K& key) const;
    V& operator[](const K& k);
    void clear();
    int32_t bucket_count() { return bucket_arr.size(); };
    int32_t size() { return inserted_n; };
    void reserve(int size);
    void erase(const K& key);
    void rehash();
    //    V& operator[](V&& k);

  private:
    int32_t DELETED = -1;
    int32_t EMPTY = -2;
    struct Element {
        Element(K key_, V* val_, int32_t hash_) : hash{hash_}, key{key_}, val{val_} {};
        Element() : key{0}, val{0}, hash{-2} {};  // fix this, or no magic. is empty
        Element(const Element& e) : hash{e.hash}, key{e.key}, val{e.val} {};
        //        ~Element(){if (val){delete val;}};
        int32_t hash;
        K key;
        V* val;
    };
    int inserted_n;
    uint64_t modulo_help;
    float lf_max;
    vector<Element> bucket_arr;
    vector<int32_t> hash_state;
    std::deque<V> valuestore;
    vector<V*> open_slots;

    int32_t hasher(const K& key) const;
    void hasher_state_gen();
    int32_t prober(const K& key, const int32_t& hash) const;
    void rehash(int size);
    std::tuple<bool, int32_t, int> contains_key(const K& key) const;
};

/*
 * constructor calls constructor with explicit size.
 * reason why i'm not doing LP(size=something) is compiler complains
 */
template <typename K, typename V>
LP<K, V>::LP() : LP{LP<K, V>(251)}
{
}
template <typename K, typename V>
LP<K, V>::LP(int size) : bucket_arr{vector<Element>(2*size)}, inserted_n{0}, modulo_help(fastmod::computeM_s32(2 * size)), lf_max{0.5}, valuestore{}, open_slots{}
{
    hasher_state_gen();
}
/*
 * function to generate hashes.
 * it's tabulation hashing.
 */
template <typename K, typename V>
int32_t LP<K, V>::hasher(const K& key) const
{
    static std::hash<K> hf;
    int32_t hash = hf(key);
    int32_t final_hash = 0;
    int32_t pos = 0;
    for (int i = 0; i < sizeof(hash); i++) {
        pos = hash & 0x00000000000000ff;
        hash = hash >> 8;
        final_hash = final_hash ^ hash_state[pos + i];
    }
    if (final_hash < 0) {
        final_hash = final_hash * -1;
    }
    return final_hash;
}
/*
 * generate random values so hasher can use them
 */
template <typename K, typename V>
void LP<K, V>::hasher_state_gen()
{
    std::vector<int32_t> state(259);
    std::generate(state.begin(), state.end(), gen_integer);
    hash_state = state;
}

/*
 * probing function.
 * it should probe locations, and stop when:
 * 1. hash = empty
 * 2. same key
 */
template <typename K, typename V>
int32_t LP<K, V>::prober(const K& key, const int32_t& hash) const
{
    unsigned long size = bucket_arr.size();
//    int32_t pos = hash % size;
    int32_t pos = fastmod::fastmod_s32(hash, modulo_help, size);
    while(bucket_arr[pos].hash != EMPTY && (bucket_arr[pos].hash != hash || bucket_arr[pos].key != key)){
        pos++;
        if (pos >= size) {
            pos -= size;
        }
    }
    return pos;
    /*
     * test to see if find_if had any perf improvement. it doesnt.
     * probably because it's hard to vectorize something which has
     * hash1, key1, ptr1, hash2, key2, ptr2 as a memory region. maybe nodemap offers better improvement.
     * at the very least, for empty checking, where you're just checking if ptr == nullptr
     */
//    auto res = std::find_if(bucket_arr.begin() + pos, bucket_arr.end(), [&, hash, key](const Element& e) {
//      return (e.hash == EMPTY || (e.hash == hash && e.key == key));
//    });
//    if (res == bucket_arr.end()) {
//        res = std::find_if(bucket_arr.begin(), bucket_arr.begin() + pos, [&, hash, key](const Element& e) {
//          return (e.hash == EMPTY || (e.hash == hash && e.key == key));
//        });
//    }
//    return res - bucket_arr.begin();
}
/*
 * returns bool, index, hash
 * probe bucket arr, and if the resulting index is empty, key doesn't exist
 */
template <typename K, typename V>
std::tuple<bool, int32_t, int> LP<K, V>::contains_key(const K& key) const
{
    int32_t hash = hasher(key);
    int pos = prober(key, hash);

    if (bucket_arr[pos].hash == EMPTY) {
        return {false, pos, hash};
    }
    return {true, pos, hash};
}

/*
 * std::unordered has this. i should probably just return contains_key()[0]
 */
template <typename K, typename V>
bool LP<K, V>::contains(const K& key) const
{
    int32_t hash = hasher(key);
    int pos = prober(key, hash);

    if (bucket_arr[pos].hash == EMPTY) {
        return false;
    }
    return true;
}
/*
 * inserts element, returns nothing.
 * detects if insertion will result in >max load factor, rehashes if it will
 * then check for existence. if there is, stop.
 * else, insert.
 *
 */
template <typename K, typename V>
void LP<K, V>::insert(const std::pair<K, V> kv)
{
    if (((inserted_n + 1) / (float)bucket_arr.size()) > lf_max) {
        rehash();
    }
    auto pos_info = contains_key(kv.first);
    if (std::get<0>(pos_info)) {
        return;
    }
    V* val_ptr;
    if (open_slots.size()) {
        val_ptr = open_slots.back();
        open_slots.pop_back();
        *val_ptr = std::move(kv.second);
    }
    else {
        valuestore.emplace_back(kv.second);
        val_ptr = &valuestore.back();
    }
    bucket_arr[std::get<1>(pos_info)] = std::move(Element{kv.first, val_ptr, std::get<2>(pos_info)});
    inserted_n++;
}
/*check for existence.
 * if there is, return value
 * if there isn't, insert V{}
 *
 */
template <typename K, typename V>
V& LP<K, V>::operator[](const K& k)
{
    auto pos_info = contains_key(k);
    if (std::get<0>(pos_info)) {
        return *bucket_arr[std::get<1>(pos_info)].val;
    }

    else {
        V* val_ptr;
        if (open_slots.size()) {
            val_ptr = open_slots.back();
            open_slots.pop_back();
        }
        else {
            valuestore.emplace_back(V{});
            val_ptr = &valuestore.back();
        }
        auto pos = std::get<1>(pos_info);
        bucket_arr[pos] = {k, val_ptr, std::get<2>(pos_info)};
        inserted_n++;
        return *bucket_arr[pos].val;
    }
}
/*
 * just delete everything
 */
template <typename K, typename V>
void LP<K, V>::clear()
{
    open_slots.clear();
    valuestore.clear();
    bucket_arr.clear();
    inserted_n = 0;
}
template <typename K, typename V>
void LP<K, V>::rehash(int size)
{
    vector<Element> arr_new(size);
    uint64_t helper = fastmod::computeM_s32(size);
    for (const auto& x : bucket_arr) {
        if (x.hash == EMPTY) {
            continue;
        }
        if (x.hash == DELETED) {
            continue;
        }
//        int32_t loc = x.hash % size;
        int32_t loc = fastmod::fastmod_s32(x.hash, helper, size);
//        TODO: check if empty check satisfies
        while (arr_new[loc].hash != EMPTY) {
            loc++;
            if (loc >= size) {
                loc -= size;
            }
        }
        arr_new[loc] = x;
    }
    bucket_arr = arr_new;
    modulo_help = helper;
}
/*
 * increase size and rehash
 */
template <typename K, typename V>
void LP<K, V>::rehash()
{
    int size = helper::next_prime(bucket_arr.size());
    rehash(size);
}

template <typename K, typename V>
void LP<K, V>::reserve(int size)
{
    int s = 1+  (size / lf_max);
    if (s < bucket_arr.size()) {
        return;
    }
    rehash(s);
}

template <typename K, typename V>
void LP<K, V>::erase(const K& key)
{
    auto pos_data = contains_key(key);
    if (!std::get<0>(pos_data)) {
        return;
    }
    auto pos = std::get<1>(pos_data);
    bucket_arr[pos].hash = DELETED;
    open_slots.push_back(bucket_arr[pos].val);
    *bucket_arr[pos].val = V{};
    bucket_arr[pos].val = nullptr;
    bucket_arr[pos].key = K{};
    inserted_n--;
}

#endif
