#ifndef NODEMAP2_H
#define NODEMAP2_H

#include <algorithm>
#include <numeric>
#include <tuple>
#include <vector>

#include "../tools/random.h"
using std::vector;
/*
 * basically a test to check if my intuition is correct.
 * checking to see if my special data structure was indeed justified.
 * it is, for <int,int>, and on my machine, for my compiler, with my compiler flags.
 *
 * this is basically basic linear probing, but instead of storing the KVpair in a vector,
 * I'm storing a vector of KVpair pointers.
 * the difference between this and nodemap is that when i insert, i just use
 * new KVpair{} and store it's pointer, while nodemap inserts it in specialized datastruct,
 * which is a lot more continuous.
 * pros:
 * 1. simpler than nodemap
 * 2. faster than std map
 * cons:
 * 2. slower than nodemap, with no obvious places where i can optimize
 *
 */

template <typename K, typename V>
class Nodemap2 {
  public:
    Nodemap2();
    explicit Nodemap2(int size);
    ~Nodemap2();

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
    struct Element {
        Element(K key_, const V val_, int32_t hash_) : hash{hash_}, key{key_}, val{val_} {};
        int32_t hash;
        K key;
        V val;
    };
    vector<Element*> bucket_arr;
    vector<int32_t> hash_state;
    int inserted_n;
    float lf_max;
    int32_t hasher(const K& key) const;
    void hasher_state_gen();
    int32_t prober(const K& key) const;
    int32_t prober(const K& key, const int32_t& hash) const;
    void rehash(int size);
    std::tuple<bool, int32_t, int> contains_key(const K& key) const;
};

template <typename K, typename V>
Nodemap2<K, V>::Nodemap2() : Nodemap2{Nodemap2<K, V>(251)}
{
}
template <typename K, typename V>
Nodemap2<K, V>::Nodemap2(int size)
    : bucket_arr{vector<Element*>(size)}, inserted_n{0}, lf_max{0.5}
{
    hasher_state_gen();
}
template <typename K, typename V>
int32_t Nodemap2<K, V>::hasher(const K& key) const
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
void Nodemap2<K, V>::hasher_state_gen()
{
    std::vector<int32_t> state(259);
    std::generate(state.begin(), state.end(), gen_integer);
    hash_state = state;
}

template <typename K, typename V>
int32_t Nodemap2<K, V>::prober(const K& key) const
{
    int32_t hash = hasher(key);
    int32_t pos = hash % bucket_arr.size();
    while (bucket_arr[pos] && bucket_arr[pos]->hash != hash  && (bucket_arr[pos]->hash == -1 ||bucket_arr[pos]->key != key)) {
        pos++;
        if (pos >= bucket_arr.size()) {
            pos -= bucket_arr.size();
        }
    }
    return pos;
}

template <typename K, typename V>
int32_t Nodemap2<K, V>::prober(const K& key, const int32_t& hash) const
{
    int32_t pos = hash % bucket_arr.size();
    while (bucket_arr[pos] && bucket_arr[pos]->hash != hash  && (bucket_arr[pos]->key != key || bucket_arr[pos]->hash == -1)) {
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
std::tuple<bool, int32_t, int> Nodemap2<K, V>::contains_key(const K& key) const
{
    int32_t hash = hasher(key);
    int pos = prober(key, hash);

    if (!bucket_arr[pos]) {
        return {false, pos, hash};
    }
    return {true, pos, hash};
}

template <typename K, typename V>
bool Nodemap2<K, V>::contains(const K& key) const
{
    int32_t hash = hasher(key);
    int pos = prober(key, hash);

    if (!bucket_arr[pos]) {
        return false;
    }
    return true;
}
template <typename K, typename V>
void Nodemap2<K, V>::insert(const std::pair<K, V> kv)
{
    if (((inserted_n + 1) / (float)bucket_arr.size()) > lf_max) {
        rehash();
    }
    auto pos_info = contains_key(kv.first);
    if (std::get<0>(pos_info)){
        return;
    }

    bucket_arr[std::get<1>(pos_info)] = new Element {kv.first, kv.second, std::get<2>(pos_info)};;
    inserted_n++;
}

template <typename K, typename V>
V& Nodemap2<K, V>::operator[](const K& k)
{
    auto pos_info = contains_key(k);
    if (std::get<0>(pos_info)){
        return bucket_arr[std::get<1>(pos_info)]->val;
    }

    else {
        auto pos = std::get<1>(pos_info);
        bucket_arr[pos] = new Element{k, V{}, std::get<2>(pos_info)};

        return bucket_arr[pos]->val;
    }
}

template <typename K, typename V>
void Nodemap2<K, V>::clear()
{
    for (auto& x : bucket_arr) {
        delete x;
        x = nullptr;
    }
    inserted_n = 0;
}
template <typename K, typename V>
void Nodemap2<K, V>::rehash(int size)
{
    vector<Element*> arr_new(size);
    for (const auto& x : bucket_arr) {
        if (!x) {
            continue;
        }
        if (x->hash == -1){
            delete x;
        }
        int32_t loc = x->hash % size;
        while (arr_new[loc] && arr_new[loc]->key != x->key) { // TODO: think if this is the correct thing
//        while (arr_new[loc]() { // TODO: replaace if eronious
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
void Nodemap2<K, V>::rehash()
{
    int size = helper::next_prime(bucket_arr.size());
    rehash(size);
}

template <typename K, typename V>
void Nodemap2<K, V>::reserve(int size)
{
    if (size < bucket_arr.size()) {
        return;
    }
    rehash(size);
}

template <typename K, typename V>
void Nodemap2<K, V>::erase(const K& key)
{
    auto pos_data = contains_key(key);
    if (!std::get<0>(pos_data)) {
        return;
    }
    auto pos = std::get<1>(pos_data);
    // TODO: maybe delete here instead of when rehashing
    bucket_arr[pos]->hash = -1;
    bucket_arr[pos]->val = V{};
    inserted_n--;
}
template <typename K, typename V>
Nodemap2<K, V>::~Nodemap2()
{
    for (auto& x : bucket_arr) {
        if (x) {
            delete x;
        }
    }
}

#endif
