#ifndef NODEMAP_H
#define NODEMAP_H

#include <algorithm>
#include <numeric>
#include <tuple>
#include <vector>

#include "helpers.h"
using std::vector;

/* This is linear probing, but ""special""
 * it's an attempt to adress the problem LPmap2 has, namely to prevent rehashes from
 * invalidating pointers to values.
 * https://en.wikipedia.org/wiki/Fundamental_theorem_of_software_engineering strikes again
 * the k,v and hash(k) are stored in a specialized construct.
 * this specialized construct does the following:
 * 1. it doesn't reallocate anything, even if it's size increases.
 * 2. be a mostly continuous memory region, to have better cache locality
 * than just allocating space in the heap ans storing their pointers.
 * My reasoning is that if i just do new KVpair and store it's pointers, KVpairs will be spread
 * all over the heap, meaning worse cache locality.
 *
 * As a test, there's nodemap2, wich just allocates randomly on the heap when needed.
 * nodemap is faster than it, but not significantly faster.
 * but on the plus side, i haven't spent time optimizing the specialized container yet.
 *
 * pros of this hashmap:
 * 1. faster than nodemap2 and standard implementation
 * 2. better memory usage than normal linear probing. well, theoretically, and only kinda.
 * cons:
 * 1. complex
 * 2. having trouble using implementing semantics, which we need for operator[const K&& key]
 *
 * ISSUES: Nodemap<string,string> doesn't work. I assume my map doesn't work for any nontrivial
 * k,v datapair.
 * FIX in next sprint.
 *
 */

/*
 * specialized, don't use for anything outside of this hashmap
 */
template <typename T>
class Cont {
  public:
    typedef T ValueType;
    Cont();
    Cont(int siz);
    T* insert(T& elem);
    void remove(T* elem);
    void reserve(int size);

  private:
    T* empty_slot_selector();
    int n_empty;
    vector<vector<T>*> store = vector<vector<T>*>(1);
    vector<vector<T*>> empty_slots;
    vector<int> segment_ends;
    vector<int> actual_store_sizes;
};

template <typename T>
Cont<T>::Cont(int size)
    : store{new vector<T>{}}, empty_slots{{}}, segment_ends{size}, n_empty{0}, actual_store_sizes{{0}}
{
    store.back()->reserve(size);
}
template <typename T>
Cont<T>::Cont() : Cont{Cont<T>(251)}
{
}

template <typename T>
T* Cont<T>::insert(T& elem)
{
    T* adress;
    if (n_empty) {
        adress = empty_slot_selector();
        *adress = elem;
        return adress;
    }
    // check if vector is in danger of reallocation
    bool is_full = (store.back()->size() == segment_ends.back());
    if (is_full) {
        int size = std::accumulate(segment_ends.begin(), segment_ends.end(), 0);
        store.push_back(new vector<T>{});
        store.back()->reserve(size);
        actual_store_sizes.push_back(0);
        empty_slots.push_back({});
        segment_ends.push_back(size);
    }
    actual_store_sizes.back()++;
    // no danger of reallocation
    store.back()->push_back(elem);
    return &(store.back()->back());
}
template <typename T>
void Cont<T>::remove(T* elem)
{
    for (int i = 0; i < store.size(); i++) {
        T* begin = store[i]->data();
        if (elem >= begin && elem <= begin + store[i]->size()) {
            empty_slots[i].push_back(elem);
            n_empty++;
            actual_store_sizes[i]--;
            if (!actual_store_sizes[i] && actual_store_sizes.size() > 1) {
                actual_store_sizes.erase(actual_store_sizes.begin() + i);
                delete store[i];
                store.erase(store.begin() + i);
                n_empty -= empty_slots[i].size();
                empty_slots.erase(empty_slots.begin() + i);
                segment_ends.erase(segment_ends.begin() + i);
            }
            return;
        }
    }
}
template <typename T>
T* Cont<T>::empty_slot_selector()
{
    int pos = 0;
    int max = 0;
    for (int i = 0; i < actual_store_sizes.size(); i++) {
        if (empty_slots[i].empty()) {
            continue;
        }
        if (actual_store_sizes[i] > max) {
            pos = i;
            max = actual_store_sizes[i];
        }
    }
    T* empty = empty_slots[pos].back();
    empty_slots[pos].pop_back();
    n_empty--;
    actual_store_sizes[pos] += 1;
    return empty;
}
template <typename T>
void Cont<T>::reserve(int size)
{
    int current = std::accumulate(segment_ends.begin(), segment_ends.end(), 0);
    if (size <= current) {
        return;
    }
    store.push_back(new vector<T>{});
    store.back()->reserve(size - current);
    actual_store_sizes.push_back(0);
    empty_slots.push_back({});
    segment_ends.push_back(size - current);
}

template <typename K, typename V>
class Nodemap {
  public:
    Nodemap();
    explicit Nodemap(int size);

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
    Cont<Element> store_elem;
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
Nodemap<K, V>::Nodemap() : Nodemap{Nodemap<K, V>(251)}
{
}
template <typename K, typename V>
Nodemap<K, V>::Nodemap(int size)
    : store_elem{Cont<Element>(size)}, bucket_arr{vector<Element*>(size)}, inserted_n{0}, lf_max{0.5}
{
    hasher_state_gen();
}
template <typename K, typename V>
int32_t Nodemap<K, V>::hasher(const K& key) const
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
void Nodemap<K, V>::hasher_state_gen()
{
    std::vector<int32_t> state(259);
    std::generate(state.begin(), state.end(), gen_integer);
    hash_state = state;
}

template <typename K, typename V>
int32_t Nodemap<K, V>::prober(const K& key) const
{
    int32_t hash = hasher(key);
    int32_t pos = hash % bucket_arr.size();
    while (bucket_arr[pos] && bucket_arr[pos]->hash != hash
           && (bucket_arr[pos]->key != key || bucket_arr[pos]->hash == -1)) {
        // TODO: verify ASAP that the line ^ is slower for other types than int. for now, uncommented is used.
        //    As soon as Cont is fixed to work with non int objects, check it.
        //    while (bucket_arr[pos] && bucket_arr[pos]->key != key) {
        pos++;
        if (pos >= bucket_arr.size()) {
            pos -= bucket_arr.size();
        }
    }
    return pos;
}

template <typename K, typename V>
int32_t Nodemap<K, V>::prober(const K& key, const int32_t& hash) const
{
    int32_t pos = hash % bucket_arr.size();
        while (bucket_arr[pos] && bucket_arr[pos]->hash != hash  && (bucket_arr[pos]->key != key ||
        bucket_arr[pos]->hash == -1)) {
//    while (bucket_arr[pos] && bucket_arr[pos]->key != key) {
        //        TODO: see todo in other prober
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
std::tuple<bool, int32_t, int> Nodemap<K, V>::contains_key(const K& key) const
{
    int32_t hash = hasher(key);
    int pos = prober(key, hash);

    if (!bucket_arr[pos]) {
        return {false, pos, hash};
    }
    return {true, pos, hash};
}

template <typename K, typename V>
bool Nodemap<K, V>::contains(const K& key) const
{
    int32_t hash = hasher(key);
    int pos = prober(key, hash);

    if (!bucket_arr[pos]) {
        return false;
    }
    return true;
}
template <typename K, typename V>
void Nodemap<K, V>::insert(const std::pair<K, V> kv)
{
    if (((inserted_n + 1) / (float)bucket_arr.size()) > lf_max) {
        rehash();
    }
    auto pos_info = contains_key(kv.first);
    if (std::get<0>(pos_info)) {
        return;
    }

    Element el{kv.first, kv.second, std::get<2>(pos_info)};
    bucket_arr[std::get<1>(pos_info)] = store_elem.insert(el);
    inserted_n++;
}

template <typename K, typename V>
V& Nodemap<K, V>::operator[](const K& k)
{
    auto pos_info = contains_key(k);
    if (std::get<0>(pos_info)) {
        return bucket_arr[std::get<1>(pos_info)]->val;
    }

    else {
        auto pos = std::get<1>(pos_info);
        Element el{k, V{}, std::get<2>(pos_info)};
        bucket_arr[pos] = store_elem.insert(el);
        return bucket_arr[pos]->val;
    }
}

template <typename K, typename V>
void Nodemap<K, V>::clear()
{
    for (auto& x : bucket_arr) {
        store_elem.remove(x);
        x = nullptr;
    }
    inserted_n = 0;
}
template <typename K, typename V>
void Nodemap<K, V>::rehash(int size)
{
    vector<Element*> arr_new(size);
    for (const auto& x : bucket_arr) {
        if (!x) {
            continue;
        }
        if (x->hash == -1) {
            continue;
        }
        int32_t loc = x->hash % size;
//        while (arr_new[loc] && arr_new[loc]->hash != x->hash) {  // TODO: think if this is the correct thing
                    while (arr_new[loc]) { // TODO: replaace if eronious
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
void Nodemap<K, V>::rehash()
{
    int size = helper::next_prime(bucket_arr.size());
    rehash(size);
}

template <typename K, typename V>
void Nodemap<K, V>::reserve(int size)
{
    if (size < bucket_arr.size()) {
        return;
    }
    rehash(size);
}

template <typename K, typename V>
void Nodemap<K, V>::erase(const K& key)
{
    auto pos_data = contains_key(key);
    if (!std::get<0>(pos_data)) {
        return;
    }
    auto pos = std::get<1>(pos_data);
    store_elem.remove(bucket_arr[pos]);

    bucket_arr[pos]->hash = -1;
    inserted_n--;
}

#endif
