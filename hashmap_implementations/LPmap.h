#ifndef LP_H
#define LP_H

#include <algorithm>
#include <deque>
#include <functional>
#include <iterator>
#include <numeric>
#include <tuple>
#include <vector>

#include "fastmod.h"
#include "helpers.h"
using std::vector;

constexpr int32_t DELETED = -1;
constexpr int32_t EMPTY = -2;

// element wrapper
template <typename K, typename V>
struct KVElement {
    KVElement(K key_, V* val_, int32_t hash_) : hash{hash_}, key{key_}, val{val_} {};
    KVElement() : key{0}, val{0}, hash{EMPTY} {};  // fix this, or no magic. is empty
    KVElement(const KVElement& e) : hash{e.hash}, key{e.key}, val{e.val} {};
    //        ~Element(){if (val){delete val;}};
    int32_t hash;
    K key;
    V* val;
};

/*
LP, but with a modulo trick
 https://github.com/lemire/fastmod
 this is just a variation of a trick you can use when you know the divisor in advance,
 eliminating divisions from it (or only need to do them once)
 It's not important to know how the modulo trick works, just that it does, and it's faster.
 LP is linear probing map.
It has a vector of <K,V*, H>, and V is stored in a deque.
 *
 */

template <typename K, typename V, typename Hash = std::hash<K>, typename Pred = std::equal_to<K>>
class LP {
    using Element = KVElement<K, V>;

  public:
    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;  // ???? still not completely sure what it does
        using value_type = Element;
        using pointer = Element*;
        using reference = Element&;
        using wrapped_vtype = std::pair<std::reference_wrapper<K>, std::reference_wrapper<V>>;
        using wrapped_pointer = wrapped_vtype*;
        using wrapped_reference = wrapped_vtype&;
        Iterator(pointer ptr) : m_ptr{ptr}, m{ptr->key, def}
        {
            if (ptr->val) {
                m = {ptr->key, *ptr->val};
            }
        };
        wrapped_reference operator*() { return m; };
        wrapped_pointer operator->() { return &m; };

        // Prefix
        Iterator& operator++()
        {
            m_ptr++;
            while (m_ptr->hash == DELETED || m_ptr->hash == EMPTY) {
                m_ptr++;
            }
            m = wrapped_vtype{m_ptr->key, *m_ptr->val};
            return *this;
        }
        // Postfix increment
        Iterator operator++(int)
        {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        // define post and pre decrement later

        friend bool operator==(const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };
        friend bool operator!=(const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; };

      private:
        pointer m_ptr;
        wrapped_vtype m;
        V def = V{};
    };
    Iterator begin() { return ++Iterator(&bucket_arr[0]); };
    Iterator end() { return Iterator(&bucket_arr.back()); }
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
    Hash hashthing;
    Pred eq;
    int inserted_n;
    uint64_t modulo_help;  // modulo trick
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
template <typename K, typename V, typename Hash, typename Pred>
LP<K, V, Hash, Pred>::LP() : LP{LP<K, V>(251)}
{
}

template <typename K, typename V, typename Hash, typename Pred>
LP<K, V, Hash, Pred>::LP(int size)
    : eq(Pred()),
      bucket_arr{vector<Element>(2 * size)},
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
 * It's not important to know how it works, just copy it, or
 * use something that works for your map.
 * one of the cuckoo hashing papers uses this.
 */
template <typename K, typename V, typename Hash, typename Pred>
int32_t LP<K, V, Hash, Pred>::hasher(const K& key) const
{
    int32_t hash = hashthing(key);
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
template <typename K, typename V, typename Hash, typename Pred>
void LP<K, V, Hash, Pred>::hasher_state_gen()
{
    std::vector<int32_t> state(259);
    std::generate(state.begin(), state.end(), gen_integer);
    hash_state = state;
    hashthing = Hash();
}

/*
 * probing function.
 * it should probe locations, and stop when:
 * 1. hash = empty
 * 2. same key.
 * it returns the position where the element is, or should be inserted.
 */
template <typename K, typename V, typename Hash, typename Pred>
int32_t LP<K, V, Hash, Pred>::prober(const K& key, const int32_t& hash) const
{
    unsigned long size = bucket_arr.size();
    //    int32_t pos = hash % size;
    int32_t pos = fastmod::fastmod_s32(hash, modulo_help, size);
    while (bucket_arr[pos].hash != EMPTY && (bucket_arr[pos].hash != hash || not eq(bucket_arr[pos].key, key))) {
        pos++;
        if (pos >= size) {
            pos -= size;
        }
    }
    return pos;
}
/*
 * returns bool exists,  int position, int hash
 * probe bucket arr, and if the resulting position is empty, key doesn't exist
 */
template <typename K, typename V, typename Hash, typename Pred>
std::tuple<bool, int32_t, int> LP<K, V, Hash, Pred>::contains_key(const K& key) const
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
 * atm, i haven't changed it, because i haven't checked if there's any perf advantage
 * in leaving it like this, eliminating 1 call to a function.
 */
template <typename K, typename V, typename Hash, typename Pred>
bool LP<K, V, Hash, Pred>::contains(const K& key) const
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
 */
template <typename K, typename V, typename Hash, typename Pred>
void LP<K, V, Hash, Pred>::insert(const std::pair<K, V> kv)
{
    if (((inserted_n + 1) / (float)bucket_arr.size()) > lf_max) {
        rehash();
    }

    auto pos_info = contains_key(kv.first);
    if (std::get<0>(pos_info)) {
        return;
    }
    V* val_ptr = nullptr;
    if (open_slots.size()) {
        val_ptr = open_slots.back();
        open_slots.pop_back();
        *val_ptr = std::move(kv.second);
    }
    else {
        valuestore.emplace_back(kv.second);
        val_ptr = &valuestore.back();
    }
    bucket_arr[std::get<1>(pos_info)] = Element{kv.first, val_ptr, std::get<2>(pos_info)};
    inserted_n++;
}

/*check for existence.
 * if there is, return value
 * if there isn't, insert V{} and return reff. to that.
 *
 */
template <typename K, typename V, typename Hash, typename Pred>
V& LP<K, V, Hash, Pred>::operator[](const K& k)
{
    auto pos_info = contains_key(k);
    if (std::get<0>(pos_info)) {
        return *bucket_arr[std::get<1>(pos_info)].val;
    }

    V* val_ptr = nullptr;
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
/*
 * just delete everything
 */
template <typename K, typename V, typename Hash, typename Pred>
void LP<K, V, Hash, Pred>::clear()
{
    open_slots.clear();
    valuestore.clear();
    bucket_arr.clear();
    inserted_n = 0;
}
/*
 * rehash the hashmap.
 * get new modulohelper thing and new array, then loop over old array
 * insert elements that aren't empty or deleted.
 */
template <typename K, typename V, typename Hash, typename Pred>
void LP<K, V, Hash, Pred>::rehash(int size)
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
 * increase size and rehash. need to add this to the public interface of LP later.
 */
template <typename K, typename V, typename Hash, typename Pred>
void LP<K, V, Hash, Pred>::rehash()
{
    int size = helper::next_prime(bucket_arr.size());
    rehash(size);
}

/*
 * reserve the hashmap for a given size.
 * this is mean you'll be able to insert <size> elements into the map
 * without rehashes.
 */
template <typename K, typename V, typename Hash, typename Pred>
void LP<K, V, Hash, Pred>::reserve(int size)
{
    int s = 1 + (size / lf_max);
    if (s < bucket_arr.size()) {
        return;
    }
    rehash(s);
}

/*
 * erase elements.
 * check if it exists, and stop if it doesnt.
 * if it does, delete.
 */
template <typename K, typename V, typename Hash, typename Pred>
void LP<K, V, Hash, Pred>::erase(const K& key)
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
