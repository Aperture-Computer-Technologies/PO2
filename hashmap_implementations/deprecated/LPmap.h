#ifndef LP_H
#define LP_H

#include <algorithm>
#include <deque>
#include <functional>
#include <iterator>
#include <numeric>
#include <set>
#include <tuple>
#include <vector>

#include "fastmod.h"
#include "helpers.h"
#include "plf_colony.h"
using std::vector;

namespace Lp {  // namespace for LP internals

    constexpr int32_t DELETED = -1;
    constexpr int32_t EMPTY = -2;
    /**
     *  @brief  wrapper for storing hashes and std::pair<K, V>*
     *  @tparam  K Key
     *  @tparam V Value
     *  @details
     * This is a struct I use instead of std::pair<int, pair<>*> for storing hashes
     * It's just here for added semantic value, plus having a guaranteed data layout I know
     *
     */
    template <typename K, typename V, template <typename> class Allocator = std::allocator>
    struct Bucket_wrapper {
        using pair = std::pair<const K, V>;
        using iter = typename plf::colony<pair, Allocator<pair>>::iterator;
        /**
         * @param hash_ hash of the key
         * @param pair_iter_ iterator to the pair
         */
        Bucket_wrapper(int32_t hash_, iter pair_iter_) : hash{hash_}, pair_iter{pair_iter_} {};
        /**
         * @brief constructor for empty bucket.
         */
        Bucket_wrapper() : hash{EMPTY}, pair_iter{} {};  //
        /**
         * @brief Copy constructor
         * @param e other Bucket_wrapper
         */
        Bucket_wrapper(const Bucket_wrapper& e) : hash{e.hash}, pair_iter{e.pair_iter} {};
        int32_t hash;
        iter pair_iter;
    };

    /**
     * @brief simple struct for passing around info needed (existence, expected/real position, and hash) when probing
     * @details
     * For passing around result data when probing the location of a KVpair.
     * This is significantly better than std::tuple. it has more semantic value, and much easier syntax
     *  (eg Result.pos instead of std::get<2>(a result tuple))
     *  it is also much better compared to having to recompute the hash and repeated probings
     */
    struct Result {
        /**
         * @brief Probing data
         * @param is_here does the key exist?
         * @param position Where does it exist, or where should it be inserted?
         * @param hash_ hash of key
         */
        Result(bool is_here, int32_t position, int hash_) : contains{is_here}, pos{position}, hash{hash_} {};
        bool contains;
        int32_t pos;
        int hash;
    };

    template <typename K, typename V, template <typename> class Allocator = std::allocator>
    class dense_iter {  // saving 4 bytes, which might make no difference, we'll see
        using Pair_elem = std::pair<const K, V>;
        using iter = typename plf::colony<Pair_elem, Allocator<Pair_elem>>::iterator;
        using group_type = typename plf::colony<Pair_elem, Allocator<Pair_elem>>::group_pointer_type;
        using skipfield_type = typename plf::colony<Pair_elem, Allocator<Pair_elem>>::skipfield_pointer_type;
        using elem_type = typename plf::colony<Pair_elem, Allocator<Pair_elem>>::aligned_pointer_type;

        Pair_elem* elem;
        group_type group;
        int32_t skipfield;

      public:
        dense_iter(iter& iter)
            : elem{reinterpret_cast<Pair_elem*>(iter.element_pointer)},
              group{iter.group_pointer},
              skipfield{(int32_t)(reinterpret_cast<intptr_t>(iter.skipfield_pointer)
                                  - reinterpret_cast<intptr_t>(iter.element_pointer))} {};
        inline dense_iter() : elem{nullptr}, group{nullptr}, skipfield{0} {};
        iter convert()
        {
            return iter(group,
                        reinterpret_cast<elem_type>(elem),
                        reinterpret_cast<skipfield_type>(skipfield + reinterpret_cast<intptr_t>(elem)));
        }
        Pair_elem* operator->() const { return elem; }
    };

}  // namespace Lp

// element wrapper

/**
 * @brief Linear probing map
 * @tparam K Key
 * @tparam V Value
 * @tparam Hash Hashing function that should be used to hash keys
 * @tparam Pred equality function to check if keys are equal
 * @tparam Allocator=std::allocator the allocator
 *
 * @details
 * LP2, but with KV in a colony.
 * https://www.plflib.org/colony.htm#faq
 * I can delegate everything to colony's delegator, no check to see if it's deleted.
 * The highlights of colony include:
 * 1. can insert without refference AND iter invalidation
 * 2. can delete without refference and iter invalidation
 * 3. performance on par with deque
 *
 * It has all the good bits of deque without the downsides.
 *
 * Using it does bring some additional downsides, however.
 * To be able to delete, I need to store an iter to it's position.
 * an iter is 24 bytes, instead of an 8-byte pointer.
 * This means each bucket wrapper is 32 bytes instead of 16.
 * I can only fit 2 buckets in a cache line.
 *
 * Consequently, probing is slower. But now, iterators actually work,
 * I'm not storing pointers to deleted pairs, and memory of the deleted pairs get released
 *
 * potential solutions:
 * 1. find a way to store extra info about the colony in the hashmap once,
 * and go back to storing pointers to pairs.
 * On deletions, I'll then have to reconstruct the iterator from the pointer and extra stored info.
 * 2. ???
 *
 */
template <typename K, typename V, typename Hash = std::hash<K>, typename Pred = std::equal_to<K>,
          template <typename> class Allocator = std::allocator>
class LP {
    using Bucket = Lp::Bucket_wrapper<K, V>;
    using Pair_elem = std::pair<const K, V>;
    using iter = typename plf::colony<Pair_elem, Allocator<Pair_elem>>::iterator;

  public:
    LP();
    explicit LP(int size);
    ~LP() { clear(); };
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

    iter begin() { return kv_store.begin(); };
    iter end() { return kv_store.end(); };

    // debugging shit
    bool filter(Bucket& b) { return b.hash > 0; }
    vector<iter> debug_iters()
    {
        vector<Bucket, Allocator<Bucket>> valid_buckets(hash_store.size());
        auto it = std::copy_if(
            hash_store.begin(), hash_store.end(), valid_buckets.begin(), [](Bucket& b) { return b.hash > 0; });
        valid_buckets.resize(std::distance(valid_buckets.begin(), it));

        vector<iter> valid_its(valid_buckets.size());
        std::transform(
            valid_buckets.begin(), valid_buckets.end(), valid_its.begin(), [](Bucket& b) { return b.pair_iter; });
        return valid_its;
    }

  private:
    Hash hashthing;
    Pred eq;  // equality operator needs to be instantiated first, for some reason
    int inserted_n;
    uint64_t modulo_help;                              // faster modulo trick thing, see lemire's fastmod
    float lf_max;                                      // max loadfactor
    vector<Bucket, Allocator<Bucket>> hash_store;      // stores <hash, kv_pair iterator>
    vector<int32_t, Allocator<int32_t>> random_state;  // random bits used for hashing
    plf::colony<Pair_elem, Allocator<Pair_elem>> kv_store;
    int32_t hasher(const K& key) const;
    void hasher_state_gen();
    inline int32_t prober(const K& key, const int32_t& hash) const;
    void rehash(int size);
    inline Lp::Result contains_key(const K& key) const;
};

/**
 * @brief default constructor
 * @tparam K = Key
 * @tparam V Value
 * @tparam Hash hashfunction
 * @tparam Pred function used to check if keys are equal
 * @tparam Allocator
 * @details
 * default constructor that delegates to constructor with explicit size.
 * reason why i'm not doing only LP(size=something) is compiler complains
 */
template <typename K, typename V, typename Hash, typename Pred, template <typename> class Allocator>
LP<K, V, Hash, Pred, Allocator>::LP() : LP{LP<K, V>(251)}
{
}

/**
 * @brief constructor where you specify size
 * @tparam K Key
 * @tparam V Value
 * @tparam Hash hash function
 * @tparam Pred key equality checker
 * @tparam Allocator allocator
 * @param size How many objects can be stored without rehash.
 * @details
 * > LP<int, int> map = LP<int, int>(1024)
 * is equivalent to
 * > LP<int, int> map{}
 * > map.reserve(1024)
 */
template <typename K, typename V, typename Hash, typename Pred, template <typename> class Allocator>
LP<K, V, Hash, Pred, Allocator>::LP(int size)
    : eq(Pred()),
      hash_store{vector<Bucket, Allocator<Bucket>>(2 * size)},
      inserted_n{0},
      modulo_help(fastmod::computeM_s32(2 * size)),
      lf_max{0.5},
      kv_store{}
{
    hasher_state_gen();
}
/**
 * @brief Function to determine the hash of the supplied key
 * @param key Key that needs to be hashed
 * @return hashed value of key
 * @details
 * function to generate hashes.
 * it's tabulation hashing.
 * It's not important to know how it works, just copy it, or
 * use something that works for your map.
 * one of the cuckoo hashing papers uses this.
 * It uses the user supplied hash and hashes that further, so even if users supply weak ass hashes,
 * we'd still get something better from it (i think/hope). I need to turn the key in a number __somehow__
 * to make tabulation hashing work.
 * Well, if i found a way to read any supplied key as an array of chars (that's fast and properly distributed),
 * i could totally ignore the user's hashing function
 */
template <typename K, typename V, typename Hash, typename Pred, template <typename> class Allocator>
int32_t LP<K, V, Hash, Pred, Allocator>::hasher(const K& key) const
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
/**
 * @brief
 * generate random values so hasher can use them
 */
template <typename K, typename V, typename Hash, typename Pred, template <typename> class Allocator>
void LP<K, V, Hash, Pred, Allocator>::hasher_state_gen()
{
    std::vector<int32_t, Allocator<int32_t>> state(259);
    std::generate(state.begin(), state.end(), gen_integer);
    random_state = std::move(state);
    hashthing = Hash();
}

/**
 *
 * @param key The key whose position should be determined
 * @param hash hash of the key
 * @return the position where it is located or should be inserted to
 * @details
 * probing function.
 * it should probe locations, and stop when:
 * 1. hash = empty
 * 2. same key.
 * it returns the position where the element is, or should be inserted.
 */
template <typename K, typename V, typename Hash, typename Pred, template <typename> class Allocator>
inline int32_t LP<K, V, Hash, Pred, Allocator>::prober(const K& key, const int32_t& hash) const
{
    unsigned long size = hash_store.size();
    int32_t pos = fastmod::fastmod_s32(hash, modulo_help, size);
    while (hash_store[pos].hash != Lp::EMPTY
           && (hash_store[pos].hash != hash || not eq(hash_store[pos].pair_iter->first, key))) {
        pos++;
        if (pos >= size) {
            pos -= size;
        }
    }
    return pos;
}

/**
 * @brief checks if key exists, for internal use only
 * @param key
 * @return Result{exists, position, hash}
 * @details
 * probe bucket arr, and if the resulting position is empty, key doesn't exist
 */
template <typename K, typename V, typename Hash, typename Pred, template <typename> class Allocator>
inline Lp::Result LP<K, V, Hash, Pred, Allocator>::contains_key(const K& key) const
{
    int32_t hash = hasher(key);
    int pos = prober(key, hash);

    if (hash_store[pos].hash == Lp::EMPTY) {
        return {false, pos, hash};
    }
    return {true, pos, hash};
}

/**
 * @brief checks if key exists
 * @param key
 * @return true/false
 * @details
 * std::unordered has this. i should probably just return contains_key()[0]
 * atm, i haven't changed it, because i haven't checked if there's any perf advantage
 * in leaving it like this, eliminating 1 call to a function.
 */
template <typename K, typename V, typename Hash, typename Pred, template <typename> class Allocator>
bool LP<K, V, Hash, Pred, Allocator>::contains(const K& key) const
{
    int32_t hash = hasher(key);
    int pos = prober(key, hash);

    if (hash_store[pos].hash == Lp::EMPTY) {
        return false;
    }
    return true;
}
/**
 *
 * @param kv Key, Value pair that needs to be inserted
 * @brief inserts kv if kv.first doesn't exist in map
 * @details
 * inserts element, returns nothing.
 * detects if insertion will result in >max load factor, rehashes if it will
 * then check for existence. if there is, stop.
 * else, insert.
 */
template <typename K, typename V, typename Hash, typename Pred, template <typename> class Allocator>
void LP<K, V, Hash, Pred, Allocator>::insert(const Pair_elem kv)
{
    if (((inserted_n + 1) / (float)hash_store.size()) > lf_max) {
        rehash();
    }
    auto pos_info = contains_key(kv.first);
    if (pos_info.contains) {
        return;
    }
    auto it = kv_store.insert(kv);
    hash_store[pos_info.pos] = Bucket{pos_info.hash, it};
    inserted_n++;
}

/**
 * @brief access operator, also inserts <key, V{}> if key doesn't exist
 * @param key
 * @return refference to value
 * @details
 * check for existence.
 * if there is, return value
 * if there isn't, insert V{} and return reff. to that.
 */
template <typename K, typename V, typename Hash, typename Pred, template <typename> class Allocator>
V& LP<K, V, Hash, Pred, Allocator>::operator[](const K& k)
{
    auto pos_info = contains_key(k);
    if (pos_info.contains) {
        return hash_store[pos_info.pos].pair_iter->second;
    }
    auto it = kv_store.insert(Pair_elem{k, V{}});
    auto pos = pos_info.pos;
    hash_store[pos] = {pos_info.hash, it};
    inserted_n++;
    return it->second;
}
/**
 * @brief deletes all keys and values, so size is 0. Beware that _capacity_ is still that of the original
 */
template <typename K, typename V, typename Hash, typename Pred, template <typename> class Allocator>
void LP<K, V, Hash, Pred, Allocator>::clear()
{
    kv_store.clear();
    hash_store.clear();
    inserted_n = 0;
}
/**
 * @brief increase the capacity such that it can contain at least size elements and rehash
 * @param size the number of keys it should be able to contain without rehashing
 * @details
 * rehash the hashmap.
 * get new modulohelper thing and new array, then loop over old array
 * insert elements that aren't empty or deleted.
 * @bug it actually doesn't respect loadfactor_max, so it will definitely rehash if you try to insert n=size elements
 */
template <typename K, typename V, typename Hash, typename Pred, template <typename> class Allocator>
void LP<K, V, Hash, Pred, Allocator>::rehash(int size)
{
    vector<Bucket, Allocator<Bucket>> arr_new(size);
    uint64_t helper = fastmod::computeM_s32(size);
    for (const auto& x : hash_store) {
        if (x.hash < 0) {
            continue;
        }
        //        int32_t loc = x.hash % size;
        int32_t loc = fastmod::fastmod_s32(x.hash, helper, size);
        while (arr_new[loc].hash != Lp::EMPTY) {
            loc++;
            if (loc >= size) {
                loc -= size;
            }
        }
        arr_new[loc] = x;
    }
    hash_store = std::move(arr_new);
    modulo_help = helper;
}
/**
 * @brief increase size and rehash. need to add this to the public interface of LP later.
 */
template <typename K, typename V, typename Hash, typename Pred, template <typename> class Allocator>
void LP<K, V, Hash, Pred, Allocator>::rehash()
{
    int size = helper::next_prime(hash_store.size());
    rehash(size);
}

/*
 * reserve the hashmap for a given size.
 * this is mean you'll be able to insert <size> elements into the map
 * without rehashes.
 */
template <typename K, typename V, typename Hash, typename Pred, template <typename> class Allocator>
void LP<K, V, Hash, Pred, Allocator>::reserve(int size)
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
template <typename K, typename V, typename Hash, typename Pred, template <typename> class Allocator>
void LP<K, V, Hash, Pred, Allocator>::erase(const K& key)
{
    auto pos_info = contains_key(key);
    if (not pos_info.contains) {
        return;
    }
    auto pos = pos_info.pos;
    hash_store[pos].hash = Lp::DELETED;
    kv_store.erase(hash_store[pos].pair_iter);
    inserted_n--;
}

#endif
