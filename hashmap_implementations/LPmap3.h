#ifndef LP3_H
#define LP3_H

#include <algorithm>
#include <deque>
#include <numeric>
#include <tuple>
#include <vector>

#include "helpers.h"
using std::vector;

/*
 * standard linear probing map.
 * It's significantly faster than std. but it won't meet some requirements of STL.
 * https://eel.is/c++draft/unord.req
 * 'The elements of an unordered associative container are organized into buckets.
 * Keys with the same hash code appear in the same bucket. Rehashing invalidates iterators,
 * changes ordering between elements, and changes which buckets elements appear in,
 * but does not invalidate pointers or references to elements.'
 * pros:
 * 1. fastest out of everything i've coded.
 * 2. simple to implement.
 * cons:
 * 1. no pointer stability
 * 2. worse memory usage than nodemaps.
 *
 */

template <typename K, typename V>
class LP3 {
  public:
    LP3();
    explicit LP3(int size);
    ~LP3() { clear(); };
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
    float lf_max;
    vector<Element> bucket_arr;
    vector<int32_t> hash_state;
    std::deque<V> valuestore;
    vector<V*> open_slots;

    int32_t hasher(const K& key) const;
    void hasher_state_gen();
    int32_t prober(const K& key) const;
    int32_t prober(const K& key, const int32_t& hash) const;
    void rehash(int size);
    std::tuple<bool, int32_t, int> contains_key(const K& key) const;
};

template <typename K, typename V>
LP3<K, V>::LP3() : LP3{LP3<K, V>(251)}
{
}
template <typename K, typename V>
LP3<K, V>::LP3(int size) : bucket_arr{vector<Element>(size)}, inserted_n{0}, lf_max{0.5}, valuestore{}, open_slots{}
{
    hasher_state_gen();
}
template <typename K, typename V>
int32_t LP3<K, V>::hasher(const K& key) const
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

template <typename K, typename V>
void LP3<K, V>::hasher_state_gen()
{
    std::vector<int32_t> state(259);
    std::generate(state.begin(), state.end(), gen_integer);
    hash_state = state;
}

template <typename K, typename V>
int32_t LP3<K, V>::prober(const K& key) const
{
    int32_t hash = hasher(key);
    int32_t pos = hash % bucket_arr.size();
    while (bucket_arr[pos].hash != EMPTY && bucket_arr[pos].hash != hash
           && (bucket_arr[pos].hash == -1 || bucket_arr[pos].key != key)) {
        pos++;
        if (pos >= bucket_arr.size()) {
            pos -= bucket_arr.size();
        }
    }
    return pos;
}

template <typename K, typename V>
int32_t LP3<K, V>::prober(const K& key, const int32_t& hash) const
{
    int32_t pos = hash % bucket_arr.size();
    while (bucket_arr[pos].hash != EMPTY && bucket_arr[pos].hash != hash
           && (bucket_arr[pos].key != key || bucket_arr[pos].hash == -1)) {
        pos++;
        if (pos >= bucket_arr.size()) {
            pos -= bucket_arr.size();
        }
    }
    return pos;
}

/*
 * returns bool, index, hash
 */
template <typename K, typename V>
std::tuple<bool, int32_t, int> LP3<K, V>::contains_key(const K& key) const
{
    int32_t hash = hasher(key);
    int pos = prober(key, hash);

    if (bucket_arr[pos].hash == EMPTY || bucket_arr[pos].hash == DELETED) {
        return {false, pos, hash};
    }
    return {true, pos, hash};
}

template <typename K, typename V>
bool LP3<K, V>::contains(const K& key) const
{
    int32_t hash = hasher(key);
    int pos = prober(key, hash);

    if (bucket_arr[pos].hash == EMPTY || bucket_arr[pos] == DELETED) {
        return false;
    }
    return true;
}
template <typename K, typename V>
void LP3<K, V>::insert(const std::pair<K, V> kv)
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
    }
    else {
        valuestore.emplace_back(kv.second);
        val_ptr = &valuestore.back();
    }
    bucket_arr[std::get<1>(pos_info)] = std::move(Element{kv.first, val_ptr, std::get<2>(pos_info)});
    inserted_n++;
}

template <typename K, typename V>
V& LP3<K, V>::operator[](const K& k)
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

        return *bucket_arr[pos].val;
    }
}

template <typename K, typename V>
void LP3<K, V>::clear()
{
    open_slots.clear();
    valuestore.clear();
    bucket_arr.clear();
    inserted_n = 0;
}
template <typename K, typename V>
void LP3<K, V>::rehash(int size)
{
    vector<Element> arr_new(size);
    for (const auto& x : bucket_arr) {
        if (x.hash == EMPTY) {
            continue;
        }
        if (x.hash == DELETED) {
            continue;
        }
        int32_t loc = x.hash % size;
        while (arr_new[loc].hash != EMPTY && arr_new[loc].key != x.key) {  // TODO: think if this is the correct thing
            loc++;
            if (loc >= size) {
                loc -= size;
            }
        }
        arr_new[loc] = x;
    }
    bucket_arr = arr_new;
}
template <typename K, typename V>
void LP3<K, V>::rehash()
{
    int size = helper::next_prime(bucket_arr.size());
    rehash(size);
}

template <typename K, typename V>
void LP3<K, V>::reserve(int size)
{
    if (size < bucket_arr.size()) {
        return;
    }
    rehash(size);
}

template <typename K, typename V>
void LP3<K, V>::erase(const K& key)
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
