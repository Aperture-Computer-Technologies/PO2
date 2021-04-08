#ifndef LP2_H
#define LP2_H

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

namespace LPspace {  // namespace for LP internals

    constexpr int32_t DELETED = -1;
    constexpr int32_t EMPTY = -2;
    /*
     * This is a struct I use instead of std::pair<int, int> for storing hashes
     * and pair<K,V>*
     * It's just here for added semantic value, plus having a guaranteed data layout I know
     */
    template <typename K, typename V>
    struct Bucket_wrapper {
        Bucket_wrapper(int32_t hash_, std::pair<K, V>* pair) : hash{hash_}, pair_p{pair} {};
        Bucket_wrapper() : hash{EMPTY}, pair_p{nullptr} {};  //
        Bucket_wrapper(const Bucket_wrapper& e) : hash{e.hash}, pair_p{e.pair_p} {};
        int32_t hash;
        std::pair<K, V>* pair_p;
    };

    /*
     * For passing around result data when probing the location of a KVpair.
     * This is significantly better than std::tuple. it has more symantic value, and much easier syntax
     *  (eg Result.pos instead of std::get<2>(a result tuple))
     *  it is also much better compared to having to recomute the hash and repeated probings
     */
    struct Result {
        Result(bool is_here, int32_t position, int hash_) : contains{is_here}, pos{position}, hash{hash_} {};
        bool contains;
        int32_t pos;
        int hash;
    };

    /*
     * The other big part of STL. Containers have iterators.
     * I'm delegating a lot to deque's iterator, because all examples I've seen of iterator implementations
     * rely on "traditional" pointer arithmic, which isn't guaranteed to work on deque, because
     * a deque is basically a bunch of arrays, and something which stores the ends of each array.
     * I cant guarantee that (pointer to an element in deque)++ will not result in an illegal memory access.
     * If the pointer points to the end of one of the internal arrays (but not the end of the queue), incrmeenting
     * and accessing it would be bad.
     * But I can't fully rely on it either, otherwise i'd just make LP.begin() return std::deque<pair>::iterator.
     * Well, I could. But deque's iterator would also iterate over deleted pairs, and I think it shouldn't for LP2
     * https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Ro-overload
     */
    template <typename K, typename V>
    class Iter {
        using iterator_category = std::bidirectional_iterator_tag;  // unordered provides forward_it,
        // but i can do both forward and backward, so why not?
        using difference_type = std::ptrdiff_t;
        using value_type = std::pair<K, V>;
        using pointer = value_type*;
        using reference = value_type&;
        using deqit = typename std::deque<value_type>::iterator;

      public:
        Iter(deqit it, vector<value_type*>* deleted_slots) : current_it{it}, current{&(*it)}, deleted{deleted_slots} {};
        reference operator*() const { return *current; }
        pointer operator->() { return current; }
        Iter& operator++();    // prefix
        Iter operator++(int);  // postfix
        Iter& operator--();
        Iter operator--(int);
        friend bool operator==(const Iter& a, const Iter& b) { return a.current == b.current; };
        friend bool operator!=(const Iter& a, const Iter& b) { return a.current != b.current; };

      private:
        pointer current;
        deqit current_it;
        vector<value_type*>* deleted;  // here to check if a pair is a deleted pair
    };
    template <typename K, typename V>
    Iter<K, V>& Iter<K, V>::operator++()
    {  // prefix
        do {
            current_it++;
            current = &(*current_it);
        } while (std::find(deleted->begin(), deleted->end(), current) != deleted->end());
        return *this;
    }
    template <typename K, typename V>
    Iter<K, V> Iter<K, V>::operator++(int)
    {  // postfix
        Iter tmp = *this;
        ++(*this);
        return tmp;
    }
    template <typename K, typename V>
    Iter<K, V>& Iter<K, V>::operator--()
    {  // prefix
        do {
            current_it--;
            current = &(*current_it);
        } while (std::find(deleted->begin(), deleted->end(), current) != deleted->end());
        return *this;
    }
    template <typename K, typename V>
    Iter<K, V> Iter<K, V>::operator--(int)
    {  // postfix
        Iter tmp = *this;
        --(*this);
        return tmp;
    }

}  // namespace LPspace

// element wrapper

/*
LP2, but with KV in deque
 *
 */

template <typename K, typename V, typename Hash = std::hash<K>, typename Pred = std::equal_to<K>,
          class Allocator = std::allocator<std::pair<K, V>>  // Yeah, no idea what I'm doing
          >
class LP2 {
    using Bucket = LPspace::Bucket_wrapper<K, V>;
    using Pair_elem = std::pair<K, V>;
    using Iter = LPspace::Iter<K, V>;
    friend class LPspace::Iter<K, V>;

  public:
    LP2();
    explicit LP2(int size);
    ~LP2() { clear(); };
    void insert(const Pair_elem kv);
    bool contains(const K& key) const;
    V& operator[](const K& k);
    void clear();
    int32_t bucket_count() { return hash_store.size(); };
    int32_t size() { return inserted_n; };
    void reserve(int size);
    void erase(const K& key);
    void rehash();
    //    V& operator[](V&& k);

    Iter begin();
    Iter end();

  private:
    Hash hashthing;
    Pred eq;
    int inserted_n;
    uint64_t modulo_help;  // modulo trick
    float lf_max;
    vector<Bucket> hash_store;
    vector<int32_t> random_state;
    std::deque<Pair_elem, Allocator> kv_store;
    vector<Pair_elem*> open_slots;

    int32_t hasher(const K& key) const;
    void hasher_state_gen();
    int32_t prober(const K& key, const int32_t& hash) const;
    void rehash(int size);
    LPspace::Result contains_key(const K& key) const;
};

/*
 * constructor calls constructor with explicit size.
 * reason why i'm not doing LP2(size=something) is compiler complains
 */
template <typename K, typename V, typename Hash, typename Pred, typename Allocator>
LP2<K, V, Hash, Pred, Allocator>::LP2() : LP2{LP2<K, V>(251)}
{
}

template <typename K, typename V, typename Hash, typename Pred, typename Allocator>
LP2<K, V, Hash, Pred, Allocator>::LP2(int size)
    : eq(Pred()),
      hash_store{vector<Bucket>(2 * size)},
      inserted_n{0},
      modulo_help(fastmod::computeM_s32(2 * size)),
      lf_max{0.5},
      kv_store{},
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
template <typename K, typename V, typename Hash, typename Pred, typename Allocator>
int32_t LP2<K, V, Hash, Pred, Allocator>::hasher(const K& key) const
{
    int32_t hash = hashthing(key);
    int32_t final_hash = 0;
    int32_t pos = 0;
    for (int i = 0; i < sizeof(hash); i++) {
        pos = hash & 0x00000000000000ff;
        hash = hash >> 8;
        final_hash = final_hash ^ random_state[pos + i];
    }
    if (final_hash < 0) {
        final_hash = final_hash * -1;
    }
    return final_hash;
}
/*
 * generate random values so hasher can use them
 */
template <typename K, typename V, typename Hash, typename Pred, typename Allocator>
void LP2<K, V, Hash, Pred, Allocator>::hasher_state_gen()
{
    std::vector<int32_t> state(259);
    std::generate(state.begin(), state.end(), gen_integer);
    random_state = state;
    hashthing = Hash();
}

/*
 * probing function.
 * it should probe locations, and stop when:
 * 1. hash = empty
 * 2. same key.
 * it returns the position where the element is, or should be inserted.
 */
template <typename K, typename V, typename Hash, typename Pred, typename Allocator>
int32_t LP2<K, V, Hash, Pred, Allocator>::prober(const K& key, const int32_t& hash) const
{
    unsigned long size = hash_store.size();
    //    int32_t pos = hash % size;
    int32_t pos = fastmod::fastmod_s32(hash, modulo_help, size);
    while (hash_store[pos].hash != LPspace::EMPTY
           && (hash_store[pos].hash != hash || not eq(hash_store[pos].pair_p->first, key))) {
        pos++;
        if (pos >= size) {
            pos -= size;
        }
    }

    return pos;
}

/*
 * returns Result(exists, position, hash)
 * probe bucket arr, and if the resulting position is empty, key doesn't exist
 */
template <typename K, typename V, typename Hash, typename Pred, typename Allocator>
LPspace::Result LP2<K, V, Hash, Pred, Allocator>::contains_key(const K& key) const
{
    int32_t hash = hasher(key);
    int pos = prober(key, hash);

    if (hash_store[pos].hash == LPspace::EMPTY) {
        return {false, pos, hash};
    }
    return {true, pos, hash};
}

/*
 * std::unordered has this. i should probably just return contains_key()[0]
 * atm, i haven't changed it, because i haven't checked if there's any perf advantage
 * in leaving it like this, eliminating 1 call to a function.
 */
template <typename K, typename V, typename Hash, typename Pred, typename Allocator>
bool LP2<K, V, Hash, Pred, Allocator>::contains(const K& key) const
{
    int32_t hash = hasher(key);
    int pos = prober(key, hash);

    if (hash_store[pos].hash == LPspace::EMPTY) {
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
template <typename K, typename V, typename Hash, typename Pred, typename Allocator>
void LP2<K, V, Hash, Pred, Allocator>::insert(const Pair_elem kv)
{
    if (((inserted_n + 1) / (float)hash_store.size()) > lf_max) {
        rehash();
    }

    auto pos_info = contains_key(kv.first);
    if (pos_info.contains) {
        return;
    }
    Pair_elem* pair_ptr = nullptr;
    if (open_slots.size()) {
        pair_ptr = open_slots.back();
        open_slots.pop_back();
        *pair_ptr = std::move(kv);
    }
    else {
        kv_store.emplace_back(kv);
        pair_ptr = &kv_store.back();
    }
    hash_store[pos_info.pos] = Bucket{pos_info.hash, pair_ptr};
    inserted_n++;
}

/*check for existence.
 * if there is, return value
 * if there isn't, insert V{} and return reff. to that.
 *
 */
template <typename K, typename V, typename Hash, typename Pred, typename Allocator>
V& LP2<K, V, Hash, Pred, Allocator>::operator[](const K& k)
{
    auto pos_info = contains_key(k);
    if (pos_info.contains) {
        return hash_store[pos_info.pos].pair_p->second;
    }

    Pair_elem* pair_ptr = nullptr;
    if (open_slots.size()) {
        pair_ptr = open_slots.back();
        open_slots.pop_back();
    }
    else {
        kv_store.emplace_back(Pair_elem{k, V{}});
        pair_ptr = &kv_store.back();
    }
    auto pos = pos_info.pos;
    hash_store[pos] = {pos_info.hash, pair_ptr};
    inserted_n++;
    return pair_ptr->second;
}
/*
 * just delete everything
 */
template <typename K, typename V, typename Hash, typename Pred, typename Allocator>
void LP2<K, V, Hash, Pred, Allocator>::clear()
{
    open_slots.clear();
    kv_store.clear();
    hash_store.clear();
    inserted_n = 0;
}
/*
 * rehash the hashmap.
 * get new modulohelper thing and new array, then loop over old array
 * insert elements that aren't empty or deleted.
 */
template <typename K, typename V, typename Hash, typename Pred, typename Allocator>
void LP2<K, V, Hash, Pred, Allocator>::rehash(int size)
{
    vector<Bucket> arr_new(size);
    uint64_t helper = fastmod::computeM_s32(size);
    for (const auto& x : hash_store) {
        if (x.hash < 0) {
            continue;
        }
        //        int32_t loc = x.hash % size;
        int32_t loc = fastmod::fastmod_s32(x.hash, helper, size);
        while (arr_new[loc].hash != LPspace::EMPTY) {
            loc++;
            if (loc >= size) {
                loc -= size;
            }
        }
        arr_new[loc] = x;
    }
    hash_store = arr_new;
    modulo_help = helper;
}
/*
 * increase size and rehash. need to add this to the public interface of LP2 later.
 */
template <typename K, typename V, typename Hash, typename Pred, typename Allocator>
void LP2<K, V, Hash, Pred, Allocator>::rehash()
{
    int size = helper::next_prime(hash_store.size());
    rehash(size);
}

/*
 * reserve the hashmap for a given size.
 * this is mean you'll be able to insert <size> elements into the map
 * without rehashes.
 */
template <typename K, typename V, typename Hash, typename Pred, typename Allocator>
void LP2<K, V, Hash, Pred, Allocator>::reserve(int size)
{
    int s = 1 + (size / lf_max);
    if (s < hash_store.size()) {
        return;
    }
    rehash(s);
}

/*
 * erase elements.
 * check if it exists, and stop if it doesnt.
 * if it does, delete.
 */
template <typename K, typename V, typename Hash, typename Pred, typename Allocator>
void LP2<K, V, Hash, Pred, Allocator>::erase(const K& key)
{
    auto pos_info = contains_key(key);
    if (not pos_info.contains) {
        return;
    }
    auto pos = pos_info.pos;
    hash_store[pos].hash = LPspace::DELETED;
    open_slots.push_back(hash_store[pos].pair_p);
    *hash_store[pos].pair_p = {K{}, V{}};
    //    auto test = std::deque<Pair_elem>::iterator(hash_store[pos].pair_p);
    hash_store[pos].pair_p = nullptr;
    inserted_n--;
}
template <typename K, typename V, typename Hash, typename Pred, typename Allocator>
LPspace::Iter<K, V> LP2<K, V, Hash, Pred, Allocator>::end()
{
    auto temp = Iter(kv_store.end(), &open_slots);
    if (std::find(open_slots.begin(), open_slots.end(), &(*temp)) != open_slots.end()) {
        temp--;
    }
    return temp;
}
template <typename K, typename V, typename Hash, typename Pred, typename Allocator>
LPspace::Iter<K, V> LP2<K, V, Hash, Pred, Allocator>::begin()
{
    auto temp = Iter(kv_store.begin(), &open_slots);
    if (std::find(open_slots.begin(), open_slots.end(), &(*temp)) != open_slots.end()) {
        temp++;
    }
    return temp;
}

#endif
