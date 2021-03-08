#ifndef NODEMAP1B_H
#define NODEMAP1B_H

#include <algorithm>
#include <deque>
#include <numeric>
#include <tuple>
#include <vector>

#include "helpers.h"
#include "specialsauce_cont.h"
using std::vector;

/*
 * Let this be a lesson to us all, especially me.
 * Look first if something already exists for what you need.
 * deque exists, does mostly what i want and probably does it better.
 * I AM PLEASED to say that deque is slightly vaster than my own container.
 * If we'll be choosing any Nodemap containers, it'll probably be this one
 */

template <typename K, typename V> class Nodemap3 {
  public:
    Nodemap3();
    explicit Nodemap3(int size);

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
    int inserted_n;
    float lf_max;
    std::deque<Element> store_elem;
    vector<Element*> bucket_arr;
    vector<int32_t> hash_state;
    vector<Element*> open_slots;
    int32_t hasher(const K& key) const;
    void hasher_state_gen();
    int32_t prober(const K& key) const;
    int32_t prober(const K& key, const int32_t& hash) const;
    void rehash(int size);
    std::tuple<bool, int32_t, int> contains_key(const K& key) const;
};

template <typename K, typename V> Nodemap3<K, V>::Nodemap3() : Nodemap3{Nodemap3<K, V>(251)} {}
template <typename K, typename V> Nodemap3<K, V>::Nodemap3(int size)
    : bucket_arr{vector<Element*>(size)}, inserted_n{0}, lf_max{0.5}, store_elem{}, open_slots{}
{
    hasher_state_gen();
}
template <typename K, typename V> int32_t Nodemap3<K, V>::hasher(const K& key) const
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

template <typename K, typename V> void Nodemap3<K, V>::hasher_state_gen()
{
    std::vector<int32_t> state(259);
    std::generate(state.begin(), state.end(), gen_integer);
    hash_state = state;
}

template <typename K, typename V> int32_t Nodemap3<K, V>::prober(const K& key) const
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

template <typename K, typename V> int32_t Nodemap3<K, V>::prober(const K& key, const int32_t& hash) const
{
    int32_t pos = hash % bucket_arr.size();
    while (bucket_arr[pos] && bucket_arr[pos]->hash != hash
           && (bucket_arr[pos]->key != key || bucket_arr[pos]->hash == -1)) {
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
template <typename K, typename V> std::tuple<bool, int32_t, int> Nodemap3<K, V>::contains_key(const K& key) const
{
    int32_t hash = hasher(key);
    int pos = prober(key, hash);

    if (!bucket_arr[pos]) {
        return {false, pos, hash};
    }
    return {true, pos, hash};
}

template <typename K, typename V> bool Nodemap3<K, V>::contains(const K& key) const
{
    int32_t hash = hasher(key);
    int pos = prober(key, hash);

    if (!bucket_arr[pos]) {
        return false;
    }
    return true;
}
template <typename K, typename V> void Nodemap3<K, V>::insert(const std::pair<K, V> kv)
{
    if (((inserted_n + 1) / (float)bucket_arr.size()) > lf_max) {
        rehash();
    }
    auto pos_info = contains_key(kv.first);
    auto pos = std::get<1>(pos_info);
    if (std::get<0>(pos_info)) {
        return;
    }
    if (open_slots.size()) {
        auto elem_ptr = open_slots.back();
        open_slots.pop_back();
        bucket_arr[pos] = elem_ptr;
        *elem_ptr = Element{kv.first, kv.second, std::get<2>(pos_info)};
    }
    else {
        store_elem.emplace_back(Element{kv.first, kv.second, std::get<2>(pos_info)});
        bucket_arr[pos] = &store_elem.back();
    }
    inserted_n++;
}

template <typename K, typename V> V& Nodemap3<K, V>::operator[](const K& k)
{
    auto pos_info = contains_key(k);
    if (std::get<0>(pos_info)) {
        return bucket_arr[std::get<1>(pos_info)]->val;
    }

    else {
        auto pos = std::get<1>(pos_info);
        store_elem.emplace_back(Element{k, V{}, std::get<2>(pos_info)});
        bucket_arr[pos] = &store_elem.back();
        return bucket_arr[pos]->val;
    }
}

template <typename K, typename V> void Nodemap3<K, V>::clear()
{
    store_elem.clear();
    bucket_arr.clear();
    open_slots.clear();
    inserted_n = 0;
}
template <typename K, typename V> void Nodemap3<K, V>::rehash(int size)
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
        while (arr_new[loc]) {  // TODO: replaace if eronious
            loc++;
            if (loc >= size) {
                loc -= size;
            }
        }
        arr_new[loc] = x;
    }
    bucket_arr = arr_new;
}
template <typename K, typename V> void Nodemap3<K, V>::rehash()
{
    int size = helper::next_prime(bucket_arr.size());
    rehash(size);
}

template <typename K, typename V> void Nodemap3<K, V>::reserve(int size)
{
    if (size < bucket_arr.size()) {
        return;
    }
    rehash(size);
}

template <typename K, typename V> void Nodemap3<K, V>::erase(const K& key)
{
    auto pos_data = contains_key(key);
    if (!std::get<0>(pos_data)) {
        return;
    }
    auto pos = std::get<1>(pos_data);
    open_slots.push_back(bucket_arr[pos]);
    bucket_arr[pos]->hash = -1;
    inserted_n--;
}

#endif
