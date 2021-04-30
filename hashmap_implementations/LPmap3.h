#ifndef LP33_H
#define LP33_H

#import <assert.h>

#include <algorithm>
#include <functional>
#include <iterator>
#include <numeric>
#include <vector>

#include "fastmod.h"
#include "helpers.h"
#include "plf_colony.h"
using std::vector;

namespace Lp {

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

    template <typename K, typename V, class Allocator = std::allocator<std::pair<const K, V>>>
    class naive_faster_colony_iter {  // saving a reinterpret cast, giving me  a 20ns speedup
        using Pair_elem = std::pair<const K, V>;
        using iter = typename plf::colony<Pair_elem, Allocator>::iterator;
        using group_type = typename plf::colony<Pair_elem>::group_pointer_type;
        using skipfield_type = typename plf::colony<Pair_elem>::skipfield_pointer_type;
        using elem_type = typename plf::colony<Pair_elem>::aligned_pointer_type;
        Pair_elem* elem;
        group_type group;
        skipfield_type skipfield;

      public:
        inline naive_faster_colony_iter(iter& iter)
            : elem{reinterpret_cast<Pair_elem*>(iter.element_pointer)},
              group{iter.group_pointer},
              skipfield{iter.skipfield_pointer} {};
        inline naive_faster_colony_iter() : elem{nullptr}, group{nullptr}, skipfield{0} {};
        inline iter convert() { return iter(group, reinterpret_cast<elem_type>(elem), skipfield); }
        inline Pair_elem* operator->() const { return elem; }
    };
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
    template <typename K, typename V>
    struct Bucket_wrapper {
        using pair = std::pair<const K, V>;
        using iter = naive_faster_colony_iter<K, V>;
        /**
         * @param hash_ hash of the key
         * @param pair_iter_ iterator to the pair
         */
        Bucket_wrapper(int32_t hash_, iter pair_iter_) : hash{hash_}, pair_iter{pair_iter_} {};
        /**
         * @brief constructor for empty bucket.
         */
        Bucket_wrapper() : hash{Lp::EMPTY}, pair_iter{} {};  //
        /**
         * @brief Copy constructor
         * @param e other Bucket_wrapper
         */
        Bucket_wrapper(const Bucket_wrapper& e) : hash{e.hash}, pair_iter{e.pair_iter} {};
        int32_t hash;
        iter pair_iter;
    };

}  // namespace Lp

/**
 * @brief Linear probing map
 * @tparam K Key
 * @tparam V Value
 * @tparam Hash Hashing function that should be used to hash keys
 * @tparam Pred equality function to check if keys are equal
 * @tparam Allocator=std::allocator the allocator
 *
 * @details
 * LP32, but with KV in a colony.
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
 * LP3.merge and LP3.extract are not implemented. These rely on the assumption that I can remove the pointer
 * to the node to my container, thereby adding/removing an element without copy/move, and leaving pointers and refs
 * intact. I could do something emulating the behaviour partially. Just insert the bucket_wrapper to the other node,
 * without touching the underlying the underlying KV pair.
 * this would be horrible due to several reasons:
 * 1. the container I extract the node from would stay the owner of that pair. if the original owner gets destructed,
 * I then have an iterator to an illegal memory space.
 * 2. Users expect a certain behaviour from this known API. breaking that would introduce subtle bugs unless they read
 * my docs carefully (and if i write the docs carefully).
 *
 * LP3.begin(int bucket) and other bucket interface iters also don't work.
 * You expect that when you iterate over these, the *it points to elements with the same hash. that's not possible here.
 *
 *
 */
template <typename K, typename V, typename Hash = std::hash<K>, typename Pred = std::equal_to<K>,
          class Allocator = std::allocator<std::pair<const K, V>>>
class LP3 {
    using Bucket = Lp::Bucket_wrapper<K, V>;
    using Pair_elem = std::pair<const K, V>;
    using plf_iter = typename plf::colony<Pair_elem, Allocator>::iterator;
    using plf_constiter = typename plf::colony<Pair_elem, Allocator>::const_iterator;

  private:
    Hash user_hash;
    Pred is_equal;
    int inserted_n;
    uint64_t modulo_help;          // faster modulo trick thing, see lemire's fastmod
    float lf_max;                  // max loadfactor
    vector<Bucket> hash_store;     // stores <hash, kv_pair iterator>
    vector<int32_t> random_state;  // random bits used for hashing
    plf::colony<Pair_elem, Allocator> kv_store;

    int32_t hasher(const K& key) const;                              // hashes key
    void hasher_state_gen();                                         // generates randomness for hashing function
    inline int32_t prober(const K& key, const int32_t& hash) const;  // probes where key should be at
    void rehash(int size);                                           // rehashes
    inline Lp::Result contains_key(const K& key) const;              // prober() with extended info

  public:
    // iterators

    class ConstIterator;  // iterator needs it to declar friend
    class Iterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = std::pair<const K, V>;
        using pointer = value_type*;
        using reference = value_type&;

        plf_iter slave;

      public:
        Iterator(plf_iter plf) : slave{plf} {};
        reference operator*() const { return *slave; }
        pointer operator->() { return slave.operator->(); }
        // Prefix increment
        Iterator& operator++()
        {
            slave++;
            return *this;
        };
        // Postfix increment
        Iterator operator++(int)
        {
            Iterator temp = *this;
            ++(*this);
            return temp;
        }
        Iterator& operator--()
        {
            slave--;
            return *this;
        };
        Iterator operator--(int)
        {
            Iterator temp = *this;
            --(*this);
            return temp;
        }

        friend bool operator==(const Iterator& a, const Iterator& b) { return a.slave == b.slave; };
        friend bool operator!=(const Iterator& a, const Iterator& b) { return a.slave != b.slave; };
        friend class ConstIterator;
    };

    class ConstIterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = std::pair<const K, V>;
        using pointer = const value_type*;
        using reference = const value_type&;

        plf_iter slave;

      public:
        ConstIterator(plf_constiter plf) : slave{plf} {};
        ConstIterator(Iterator lp3_it) : slave{lp3_it.slave} {};
        reference operator*() const { return *slave; }
        pointer operator->() { return slave.operator->(); }
        // Prefix increment
        ConstIterator& operator++()
        {
            slave++;
            return *this;
        };
        // Postfix increment
        ConstIterator operator++(int)
        {
            ConstIterator temp = *this;
            ++(*this);
            return temp;
        }
        ConstIterator& operator--()
        {
            slave--;
            return *this;
        };
        ConstIterator operator--(int)
        {
            ConstIterator temp = *this;
            --(*this);
            return temp;
        }

        friend bool operator==(const ConstIterator& a, const ConstIterator& b) { return a.slave == b.slave; };
        friend bool operator!=(const ConstIterator& a, const ConstIterator& b) { return a.slave != b.slave; };
    };

    Iterator begin() { return Iterator(kv_store.begin()); };
    Iterator end() { return Iterator(kv_store.end()); };
    ConstIterator cbegin() const { return ConstIterator(kv_store.cbegin()); };
    ConstIterator cend() const { return ConstIterator(kv_store.cend()); };

    // constructors and destructors
    LP3();
    explicit LP3(size_t bucket_count);
    LP3(Iterator first, Iterator last);
    LP3(Iterator first, Iterator last, size_t size);
    //    below constructors are correct, ^ may need attention due to allocator changes
    LP3(const LP3& other);  // copy constructor
    LP3(LP3&& other);       // move constructor
    LP3(std::initializer_list<Pair_elem> init);
    LP3(std::initializer_list<Pair_elem> init, size_t bucket_count);
    ~LP3() { clear(); };
    LP3& operator=(const LP3& other);                        // copy assignment
    LP3& operator=(std::initializer_list<Pair_elem> ilist);  // assign init list

#if __cplusplus >= 201703L  // move assignment
    LP3& operator=(LP3&& other) noexcept;
#else
    LP3& operator=(LP3&& other);
#endif
    Allocator get_allocator() const noexcept { return Allocator(); };

        // Capacity checks
#if __cplusplus >= 202002L  // if c++ version > c++20
    [[nodiscard]] bool empty() const noexcept { return kv_store.empty(); };
#else
    bool empty() const noexcept { return kv_store.empty(); };
#endif
    int32_t size() const { return inserted_n; };
    // theoretical max elements you could have in it
    size_t max_size() const noexcept { return std::min({kv_store.max_size(), hash_store.max_size()}); }

    // --------------- modifying stuff
    void clear() noexcept;
    // insertion overloads. all overloads that delegate are inlined
    std::pair<Iterator, bool> insert(const Pair_elem& value);            // copy
    inline Iterator insert(ConstIterator hint, const Pair_elem& value);  // copy, delegate to above
    template <class P>
    inline std::pair<Iterator, bool> insert(P&& value);  // delegate to move insert
    template <class P>
    inline Iterator insert(ConstIterator hint, P&& value);  // delegate to move insert or to above
    /* TODO: the above are meant to eliminate a move/copy. instead of convert, copy/move, it should do convert
     * atm, they don't do that, but they do correctly insert
     */
    // construction only
    template <class InputIt>
    inline void insert(InputIt first, InputIt last);                      // delegate
    inline void insert(std::initializer_list<Pair_elem> ilist);           // delegate
#if __cplusplus >= 201703L                                                // if C++17+
    std::pair<Iterator, bool> insert(Pair_elem&& value);                  // move
    inline Iterator insert(ConstIterator hint, const Pair_elem&& value);  // move, delegate to above
    using std_nodetype = typename std::unordered_map<K, V>::node_type;
    std::pair<Iterator, bool> insert(std_nodetype&& nh);                      // insert from unordered_map::node
    std::pair<Iterator, bool> insert(ConstIterator hint, std_nodetype&& nh);  // delegate to above
#else
#endif

    //    insert or assign overloads
#if __cplusplus >= 201703L  // if C++17+
    template <class M>
    std::pair<Iterator, bool> insert_or_assign(const K& k, M&& obj);  // copy key
    template <class M>
    std::pair<Iterator, bool> insert_or_assign(K&& k, M&& obj);  // move key
    template <class M>
    inline Iterator insert_or_assign(ConstIterator hint, const K& k, M&& obj);
    template <class M>
    inline Iterator insert_or_assign(ConstIterator hint, K&& k, M&& obj);
#else
#endif

    //    emplace. They work in the sense that they do insert when needed
    //    they don't work in the sense that it's NEVER more efficient than insert
    //     TODO: change that sometime
    template <class... Args>
    std::pair<Iterator, bool> emplace(Args&&... args);
    template <class... Args>
    Iterator emplace_hint(ConstIterator hint, Args&&... args);
    //    try emplace for some other day https://en.cppreference.com/w/cpp/container/unordered_map/try_emplace

#if __cplusplus >= 201703L
    void swap(LP3&) noexcept;
#else
    void swap(LP3& other);
#endif

    void erase(const K& key);

    //   modifying lookups
    V& operator[](const K& k);
    V& operator[](K&& k);
    // nonmodifying lookups
    V& at(const K& k);
    const V& at(const K& k) const;
    size_t count(const K& key) const;
    Iterator find(const K& key);
    ConstIterator find(const K& key) const;
//    template< class K > iterator find( const K& x ); //since C++20)
//    template< class K > const_iterator find( const K& x ) const; //(since C++20)
#if __cplusplus >= 202002L
    bool contains(const K& key) const;
//    template< class Key >
//    bool contains( const Key& x ) const;
#else
#endif
    std::pair<Iterator, Iterator> equal_range(const K& key);
    std::pair<ConstIterator, ConstIterator> equal_range(const K& key) const;

    //    bucket interface
    int32_t bucket_count() const { return hash_store.size(); };
    int32_t max_bucket_count() const { return max_size(); };
    size_t bucket_size(size_t n) const { return (kv_store[n].hash >= 0); };
    size_t bucket(const K& key) const { return contains_key(key).pos; };

    //    Hash policy
    float load_factor() const { return kv_store.size() / inserted_n; };
    float max_load_factor() const { return lf_max; };
    void max_load_factor(float ml);
    void rehash();
    void reserve(int size);

    //     observers
    Hash hash_function() const { return hasher; };
    Pred key_eq() const { return key_eq; };
};

// --------- private functions

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
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
int32_t LP3<K, V, Hash, Pred, Allocator>::hasher(const K& key) const
{
    int32_t hash = user_hash(key);
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
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
void LP3<K, V, Hash, Pred, Allocator>::hasher_state_gen()
{
    std::vector<int32_t> state(259);
    std::generate(state.begin(), state.end(), gen_integer);
    random_state = std::move(state);
    user_hash = Hash();
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
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
inline int32_t LP3<K, V, Hash, Pred, Allocator>::prober(const K& key, const int32_t& hash) const
{
    unsigned long size = hash_store.size();
    int32_t pos = fastmod::fastmod_s32(hash, modulo_help, size);
    while (hash_store[pos].hash != Lp::EMPTY
           && (hash_store[pos].hash != hash || not is_equal(hash_store[pos].pair_iter->first, key))) {
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
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
inline Lp::Result LP3<K, V, Hash, Pred, Allocator>::contains_key(const K& key) const
{
    int32_t hash = hasher(key);
    int pos = prober(key, hash);

    if (hash_store[pos].hash == Lp::EMPTY) {
        return {false, pos, hash};
    }
    return {true, pos, hash};
}
//--------------------------- END PRIVATE FUNCTIONS

/**
 * @brief default constructor
 * @tparam K = Key
 * @tparam V Value
 * @tparam Hash hashfunction
 * @tparam Pred function used to check if keys are equal
 * @tparam Allocator
 * @details
 * default constructor that delegates to constructor with explicit size.
 * reason why i'm not doing only LP3(size=something) is compiler complains
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
LP3<K, V, Hash, Pred, Allocator>::LP3() : LP3{LP3<K, V>(251)}
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
 * > LP3<int, int> map = LP3<int, int>(1024)
 * is equivalent to
 * > LP3<int, int> map{}
 * > map.reserve(1024)
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
LP3<K, V, Hash, Pred, Allocator>::LP3(size_t size)
    : is_equal(Pred()),
      inserted_n{0},
      modulo_help(fastmod::computeM_s32(helper::next_prime(2 * size))),
      lf_max{0.5},
      hash_store{vector<Bucket>(helper::next_prime(2 * size))},
      kv_store{}
{
    hasher_state_gen();
}

/**
 *
 * @param first iterator to first element you want to include
 * @param last iterator to last element of the range you want to include
 * Constructs LP3 from another (part of) LP3 with iterators
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
LP3<K, V, Hash, Pred, Allocator>::LP3(LP3::Iterator first, LP3::Iterator last) : LP3{LP3<K, V>(251)}
{
    while (first != last) {
        insert(*first++);
    }
}

/**
 *
 * @param first iterator to first element you want to include
 * @param last iterator to last element of the range you want to include
 * Constructs LP3 from another (part of) LP3 with iterators, but you can specify size
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
LP3<K, V, Hash, Pred, Allocator>::LP3(LP3::Iterator first, LP3::Iterator last, size_t size) : LP3{LP3<K, V>(size)}
{
    while (first != last) {
        insert(*first++);
    }
}

/**
 * @details Copy constructor
 * @param other Other hashmap you want to copy
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
LP3<K, V, Hash, Pred, Allocator>::LP3(const LP3& other)
    : is_equal(other.is_equal),
      inserted_n{other.inserted_n},
      modulo_help(other.modulo_help),
      lf_max{other.lf_max},
      hash_store{other.hash_store},
      random_state{other.random_state},
      kv_store{other.kv_store} {};

/**
 * @details Move constructor
 * @param other Other hashmap you want to move
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
LP3<K, V, Hash, Pred, Allocator>::LP3(LP3&& other)
{
    other.swap(*this);
    other.clear();
    //    std::swap(user_hash, other.user_hash);
    //    std::swap(is_equal, other.is_equal);
    //    std::swap(inserted_n, other.inserted_n);
    //    std::swap(modulo_help, other.modulo_help);
    //    std::swap(lf_max, other.lf_max);
    //    std::swap(hash_store, other.hash_store);
    //    std::swap(random_state, other.random_state);
    //    std::swap(kv_store, other.kv_store);
    //    other.clear();
};

template <typename K, typename V, typename Hash, typename Pred, class Allocator>
LP3<K, V, Hash, Pred, Allocator>::LP3(std::initializer_list<Pair_elem> init) : LP3{LP3<K, V>{}}
{
    for (const auto& x : init) {
        insert(x);
    }
}

template <typename K, typename V, typename Hash, typename Pred, class Allocator>
LP3<K, V, Hash, Pred, Allocator>::LP3(std::initializer_list<Pair_elem> init, size_t bucket_count)
    : LP3{LP3<K, V>(bucket_count)}
{
    for (const auto& x : init) {
        insert(x);
    }
}

// ---------------------- end constructors
//----------------------- begin assignment

/**
 *
 * @param other map you want to copy assign
 * @return this with the new state
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
LP3<K, V, Hash, Pred, Allocator>& LP3<K, V, Hash, Pred, Allocator>::operator=(const LP3& other)
{
    auto temp{other};
    swap(*this, temp);
    return *this;
}

/**
 *
 * @param other map you want to move assign
 * @return this with the new state
 */
#if __cplusplus >= 201703L
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
LP3<K, V, Hash, Pred, Allocator>& LP3<K, V, Hash, Pred, Allocator>::operator=(LP3&& other) noexcept
{
    swap(*this, other);
    return *this;
}
#else
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
LP3<K, V, Hash, Pred, Allocator>& LP3<K, V, Hash, Pred, Allocator>::operator=(LP3&& other)
{
    swap(*this, other);
    return *this;
}
#endif

template <typename K, typename V, typename Hash, typename Pred, class Allocator>
LP3<K, V, Hash, Pred, Allocator>& LP3<K, V, Hash, Pred, Allocator>::operator=(std::initializer_list<Pair_elem> ilist)
{
    auto temp = LP3{ilist};
    temp.swap(*this);
    return *this;
}

// ------------------- end assignment ------------------
// ------------------- begin modifying functions

/**
 * @brief deletes all keys and values, so size is 0. Beware that _capacity_ is still that of the original
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
void LP3<K, V, Hash, Pred, Allocator>::clear() noexcept
{
    kv_store.clear();
    hash_store.clear();
    inserted_n = 0;
}

// ------------------- begin insert overloads
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
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
std::pair<typename LP3<K, V, Hash, Pred, Allocator>::Iterator, bool> LP3<K, V, Hash, Pred, Allocator>::insert(
    const LP3::Pair_elem& kv)
{
    if (((inserted_n + 1) / (float)hash_store.size()) > lf_max) {
        rehash();
    }
    auto pos_info = contains_key(kv.first);
    if (pos_info.contains) {
        return {hash_store[pos_info.pos].pair_iter.convert(), false};
    }
    auto it = kv_store.insert(kv);
    hash_store[pos_info.pos] = Bucket{pos_info.hash, it};
    inserted_n++;
    return std::pair<Iterator, bool>(it, true);
}

#if __cplusplus >= 201703L  // if C++17+

template <typename K, typename V, typename Hash, typename Pred, class Allocator>
std::pair<typename LP3<K, V, Hash, Pred, Allocator>::Iterator, bool> LP3<K, V, Hash, Pred, Allocator>::insert(
    LP3::Pair_elem&& kv)
{
    if (((inserted_n + 1) / (float)hash_store.size()) > lf_max) {
        rehash();
    }
    auto pos_info = contains_key(kv.first);
    if (pos_info.contains) {
        return {hash_store[pos_info.pos].pair_iter.convert(), false};
    }
    auto it = kv_store.insert(std::forward<Pair_elem&&>(kv));
    hash_store[pos_info.pos] = Bucket{pos_info.hash, it};
    inserted_n++;
    return std::pair<Iterator, bool>(it, true);
}
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
typename LP3<K, V, Hash, Pred, Allocator>::Iterator LP3<K, V, Hash, Pred, Allocator>::insert(ConstIterator hint,
                                                                                             const Pair_elem&& kv)
{
    return insert(std::forward<Pair_elem&&>(kv));
}

template <typename K, typename V, typename Hash, typename Pred, class Allocator>
std::pair<typename LP3<K, V, Hash, Pred, Allocator>::Iterator, bool> LP3<K, V, Hash, Pred, Allocator>::insert(
    std_nodetype&& node)
{
    if (node) {
        Pair_elem kv = {std::move(node.key), std::move(node.value)};
        if (((inserted_n + 1) / (float)hash_store.size()) > lf_max) {
            rehash();
        }
        auto pos_info = contains_key(kv.first);
        if (pos_info.contains) {
            return {hash_store[pos_info.pos].pair_iter.convert(), false};
        }
        auto it = kv_store.insert(std::forward<Pair_elem&&>(kv));
        hash_store[pos_info.pos] = Bucket{pos_info.hash, it};
        inserted_n++;
        return std::pair<Iterator, bool>(it, true);
    }
    return {end(), false};
}

template <typename K, typename V, typename Hash, typename Pred, class Allocator>
std::pair<typename LP3<K, V, Hash, Pred, Allocator>::Iterator, bool> LP3<K, V, Hash, Pred, Allocator>::insert(
    ConstIterator hint, std_nodetype&& node)
{
    return insert(std::forward<std_nodetype&&>(node));
}

#else
#endif

template <typename K, typename V, typename Hash, typename Pred, class Allocator>
typename LP3<K, V, Hash, Pred, Allocator>::Iterator LP3<K, V, Hash, Pred, Allocator>::insert(ConstIterator hint,
                                                                                             const Pair_elem& kv)
{
    return insert(kv);
}

template <typename K, typename V, typename Hash, typename Pred, class Allocator>
template <class P>
std::pair<typename LP3<K, V, Hash, Pred, Allocator>::Iterator, bool> LP3<K, V, Hash, Pred, Allocator>::insert(P&& value)
{
    return insert(std::forward<Pair_elem&&>(Pair_elem{value}));
}

template <typename K, typename V, typename Hash, typename Pred, class Allocator>
template <class P>
typename LP3<K, V, Hash, Pred, Allocator>::Iterator LP3<K, V, Hash, Pred, Allocator>::insert(ConstIterator hint,
                                                                                             P&& value)
{
    return insert(hint, std::forward<Pair_elem&&>(Pair_elem{value}));
}

template <typename K, typename V, typename Hash, typename Pred, class Allocator>
template <class InputIt>
void LP3<K, V, Hash, Pred, Allocator>::insert(InputIt first, InputIt last)
{
    while (first != last) {
        insert(*first++);
    }
}
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
void LP3<K, V, Hash, Pred, Allocator>::insert(std::initializer_list<Pair_elem> ilist)
{
    for (auto x : ilist) {
        insert(std::move(x));
    }
}

// --------------------- end insert overloads

// --------------------- begin insert_or_assign_overloads

#if __cplusplus >= 201703L
/**
 *
 * @tparam K
 * @tparam V
 * @tparam Hash
 * @tparam Pred
 * @tparam Allocator
 * @tparam M type that's assignable to K
 * @param k key
 * @param obj value
 * @return pair<iterator, is inserted>
 * @details
 If a key equivalent to k already exists in the container,
 assigns std::forward<M>(obj) to the mapped_type
 corresponding to the key k. If the key does not exist,
 inserts the new value as if by insert,
 constructing it from value_type(k, std::forward<M>(obj))
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
template <class M>
std::pair<typename LP3<K, V, Hash, Pred, Allocator>::Iterator, bool> LP3<K, V, Hash, Pred, Allocator>::insert_or_assign(
    const K& k, M&& obj)
{
    auto pos_info = contains_key(k);
    if (pos_info.contains) {
        auto it = hash_store[pos_info.pos].pair_iter;
        it->second = std::forward<M>(obj);
        return {it.convert(), false};
    }
    else {
        auto it = kv_store.insert({k, std::forward<M>(obj)});
        hash_store[pos_info.pos] = {pos_info.hash, it};
        return std::pair<Iterator, bool>(it, true);
    }
}

/**
 * @details
 * creates a pair from (std::move(k), std::forward<M>(obj)) when constructed
 * assigns std::forward<M>(obj) to pair.second if assigned
 * @return <iterator to modified location, bool is_inserted>
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
template <class M>
std::pair<typename LP3<K, V, Hash, Pred, Allocator>::Iterator, bool> LP3<K, V, Hash, Pred, Allocator>::insert_or_assign(
    K&& k, M&& obj)
{
    auto pos_info = contains_key(k);
    if (pos_info.contains) {
        auto it = hash_store[pos_info.pos].pair_iter;
        it->second = std::forward<M>(obj);
        return it.convert();
    }
    else {
        auto it = kv_store.insert({std::move(k), std::forward<M>(obj)});
        hash_store[pos_info.pos] = {pos_info.hash, it};
        return it;
    }
}

/**
 * @details
 If a key equivalent to k already exists in the container,
 assigns std::forward<M>(obj) to the mapped_type
 corresponding to the key k. If the key does not exist,
 inserts the new value as if by insert,
 constructing it from value_type(k, std::forward<M>(obj))
 * @return iterator to modified location
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
template <class M>
typename LP3<K, V, Hash, Pred, Allocator>::Iterator LP3<K, V, Hash, Pred, Allocator>::insert_or_assign(ConstIterator it,
                                                                                                       const K& k,
                                                                                                       M&& obj)
{
    return insert_or_assign(k, std::forward<M&&>(obj)).first;
}

/**
 *
 * @details
 * creates a pair from (std::move(k), std::forward<M>(obj)) when constructed
 * assigns std::forward<M>(obj) to pair.second if assigned
 * @return iterator to modified location
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
template <class M>
typename LP3<K, V, Hash, Pred, Allocator>::Iterator LP3<K, V, Hash, Pred, Allocator>::insert_or_assign(
    ConstIterator hint, K&& k, M&& obj)
{
    return insert_or_assign(std::forward<K&&>(k), std::forward<M&&>(obj)).first;
}
#else
#endif

// -------------------- end insert or assign overloads

/**
 * @details
 * Inserts a new element into the container constructed in-place with the given args if there is no element with the key
 * in the container. The element may be constructed even if there already is an element with the key in the container,
 * in which case the newly constructed element will be destroyed immediately. Currently, it IS always constructed
 * regardless if it exists.
 * @return  pair(iter to inserted, bool inserted)
 *
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
template <class... Args>
std::pair<typename LP3<K, V, Hash, Pred, Allocator>::Iterator, bool> LP3<K, V, Hash, Pred, Allocator>::emplace(
    Args&&... args)
{
    //    TODO: remove the guaranteed instantiation, use
    //    http://ldionne.com/2015/11/29/efficient-parameter-pack-indexing/
    std::pair<Args...> temp{args...};
    bool constructible = std::is_constructible<Pair_elem, std::pair<Args...>>::value;
    assert(constructible);
    return insert(std::forward<Pair_elem&&>(temp));
}

template <typename K, typename V, typename Hash, typename Pred, class Allocator>
template <class... Args>
typename LP3<K, V, Hash, Pred, Allocator>::Iterator LP3<K, V, Hash, Pred, Allocator>::emplace_hint(ConstIterator hint,
                                                                                                   Args&&... args)
{
    //    TODO: remove the guaranteed instantiation, use
    //    http://ldionne.com/2015/11/29/efficient-parameter-pack-indexing/
    std::pair<Args...> temp{args...};
    bool constructible = std::is_constructible<Pair_elem, std::pair<Args...>>::value;
    assert(constructible);
    return insert(std::forward<Pair_elem&&>(temp));
}

/**
 * @details swap 2 hashmaps with each other
 */
#if __cplusplus >= 201703L
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
void LP3<K, V, Hash, Pred, Allocator>::swap(LP3& other) noexcept
{
    std::swap(user_hash, other.user_hash);
    std::swap(is_equal, other.is_equal);
    std::swap(inserted_n, other.inserted_n);
    std::swap(modulo_help, other.modulo_help);
    std::swap(lf_max, other.lf_max);
    std::swap(hash_store, other.hash_store);
    std::swap(random_state, other.random_state);
    std::swap(kv_store, other.kv_store);
    return;
}
#else
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
void LP3<K, V, Hash, Pred, Allocator>::swap(LP3& other)
{
    std::swap(user_hash, other.user_hash);
    std::swap(is_equal, other.is_equal);
    std::swap(inserted_n, other.inserted_n);
    std::swap(modulo_help, other.modulo_help);
    std::swap(lf_max, other.lf_max);
    std::swap(hash_store, other.hash_store);
    std::swap(random_state, other.random_state);
    std::swap(kv_store, other.kv_store);
    return;
}
#endif

// -------------- begin lookups

/**
 * @brief access operator, also inserts <key, V{}> if key doesn't exist
 * @param key
 * @return refference to value
 * @details
 * check for existence.
 * if there is, return value
 * if there isn't, insert V{} and return reff. to that.
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
V& LP3<K, V, Hash, Pred, Allocator>::operator[](const K& k)
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
 * @brief access operator, also inserts <key, V{}> if key doesn't exist
 * @param key
 * @return refference to value
 * @details
 * check for existence.
 * if there is, return value
 * if there isn't, insert V{} and return reff. to that.
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
V& LP3<K, V, Hash, Pred, Allocator>::operator[](K&& k)
{
    auto pos_info = contains_key(k);
    if (pos_info.contains) {
        return hash_store[pos_info.pos].pair_iter->second;
    }
    auto it = kv_store.insert(Pair_elem{std::forward<K&&>(k), V{}});
    auto pos = pos_info.pos;
    hash_store[pos] = {pos_info.hash, it};
    inserted_n++;
    return it->second;
}
// --------------------- end modifying functions
/**
 *
 * @param k key
 * @return V refference to value
 * @details Returns a reference to the mapped value of the element with key equivalent to key.
 * If no such element exists, an exception of type std::out_of_range is thrown.
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
V& LP3<K, V, Hash, Pred, Allocator>::at(const K& k)
{
    auto pos_info = contains_key(k);
    if (pos_info.contains) {
        return hash_store[pos_info.pos].pair_it->second;
    }
    else {
        throw std::out_of_range("key doesn't exist");
    }
}

/**
 *
 * @param k key
 * @return const V& const reference to value if it exists
 * @details Returns a reference to the mapped value of the element with key equivalent to key.
 * If no such element exists, an exception of type std::out_of_range is thrown.
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
const V& LP3<K, V, Hash, Pred, Allocator>::at(const K& k) const
{
    auto pos_info = contains_key(k);
    if (pos_info.contains) {
        return hash_store[pos_info.pos].pair_it->second;
    }
    else {
        throw std::out_of_range("key doesn't exist");
    }
}

/**
 * @details returns 1 if key exists, 0 otherwise
 * @return 1 if key exists, 0 otherwise
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
size_t LP3<K, V, Hash, Pred, Allocator>::count(const K& key) const
{
    return contains_key(key).contains;
}
/**
 * @param key key to find
 * @return iterator to key if exists, LP3.end() if it doesn't
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
typename LP3<K, V, Hash, Pred, Allocator>::Iterator LP3<K, V, Hash, Pred, Allocator>::find(const K& key)
{
    auto pos_info = contains_key(key);
    if (pos_info.contains) {
        return hash_store[pos_info.pos].pair_iter.convert();
    }
    return kv_store.end();
}

/**
 * @param key key to find
 * @return iterator to key if exists, LP3.end() if it doesn't
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
typename LP3<K, V, Hash, Pred, Allocator>::ConstIterator LP3<K, V, Hash, Pred, Allocator>::find(const K& key) const
{
    auto pos_info = contains_key(key);
    if (pos_info.contains) {
        return hash_store[pos_info.pos].pair_iter.convert();
    }
    return kv_store.cend();
}

#if __cplusplus >= 202002L
/**
 * @brief checks if key exists
 * @param key
 * @return true/false
 * @details
 * std::unordered has this. i should probably just return contains_key()[0]
 * atm, i haven't changed it, because i haven't checked if there's any perf advantage
 * in leaving it like this, eliminating 1 call to a function.
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
bool LP3<K, V, Hash, Pred, Allocator>::contains(const K& key) const
{
    int32_t hash = hasher(key);
    int pos = prober(key, hash);

    if (hash_store[pos].hash == Lp::EMPTY) {
        return false;
    }
    return true;
}
#else
#endif

/**
 *
 * @param key to find
 * @return std::pair containing a pair of iterators defining the wanted range. If there are no such elements,
 * past-the-end iterators are returned as both elements of the pair.
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
std::pair<typename LP3<K, V, Hash, Pred, Allocator>::Iterator, typename LP3<K, V, Hash, Pred, Allocator>::Iterator>
LP3<K, V, Hash, Pred, Allocator>::equal_range(const K& key)
{
    Iterator first = find(key);
    if (first == end()) {
        return {first, first};
    }
    return {first, ++first};
}

/**
 *
 * @param key to find
 * @return std::pair containing a pair of const iterators defining the wanted range. If there are no such elements,
 * past-the-end iterators are returned as both elements of the pair.
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
std::pair<typename LP3<K, V, Hash, Pred, Allocator>::ConstIterator,
          typename LP3<K, V, Hash, Pred, Allocator>::ConstIterator>
LP3<K, V, Hash, Pred, Allocator>::equal_range(const K& key) const
{
    ConstIterator first = find(key);
    if (first == cend()) {
        return {first, first};
    }
    return {first, ++first};
}
/**
 *
 * @param ml new max loadfactor
 * @throws std::out_of_range if ml > 1
 * @details
 * Manages the maximum load factor (number of elements per bucket).
 * The container automatically increases the number of buckets if the load factor exceeds this threshold.
 *
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
void LP3<K, V, Hash, Pred, Allocator>::max_load_factor(float ml)
{
    if (ml > 1) {
        throw std::out_of_range("max loadfactor is 1");
    }
    lf_max = ml;
    if (kv_store.size() * lf_max > inserted_n) {
        rehash();
    }
}

/// ------------------ end lookups
/**
 * @brief increase the capacity such that it can contain at least size elements and rehash
 * @param size the number of keys it should be able to contain without rehashing
 * @details
 * rehash the hashmap.
 * get new modulohelper thing and new array, then loop over old array
 * insert elements that aren't empty or deleted.
 * @bug it actually doesn't respect loadfactor_max, so it will definitely rehash if you try to insert n=size elements
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
void LP3<K, V, Hash, Pred, Allocator>::rehash(int size)
{
    vector<Bucket> arr_new(size);
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
 * @brief increase size and rehash. need to add this to the public interface of LP3 later.
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
void LP3<K, V, Hash, Pred, Allocator>::rehash()
{
    int size = helper::next_prime(int(kv_store.size() / lf_max));
    rehash(size);
}

/*
 * reserve the hashmap for a given size.
 * this is mean you'll be able to insert <size> elements into the map
 * without rehashes.
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
void LP3<K, V, Hash, Pred, Allocator>::reserve(int size)
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
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
void LP3<K, V, Hash, Pred, Allocator>::erase(const K& key)
{
    auto pos_info = contains_key(key);
    if (not pos_info.contains) {
        return;
    }
    auto pos = pos_info.pos;
    hash_store[pos].hash = Lp::DELETED;
    kv_store.erase(hash_store[pos].pair_iter.convert());
    inserted_n--;
}

/**
 *
 * @param lhs left lp3 map
 * @param rhs right lp3 map
 * @return bool isequal
 * @details
 * The contents of two unordered containers lhs and rhs are equal if the following conditions hold:
    - lhs.size() == rhs.size()
    - each key, value pair in lhs has the same key, value pair in rhs
The behavior is undefined if Key or T are not EqualityComparable.
 *
 */
template <class K_, class V_, class Hash_, class Pred_, class Allocator_>
bool operator==(const LP3<K_, V_, Hash_, Pred_, Allocator_>& lhs, const LP3<K_, V_, Hash_, Pred_, Allocator_>& rhs)
{
    if (&lhs == &rhs) {
        return true;
    }
    if (lhs.size() != rhs.size()) {
        return false;
    }
    auto it = lhs.cbegin();
    while (it != lhs.cend()) {
        auto rhs_it = rhs.find(it->first);
        if (rhs_it->second != it->second) {
            return false;
        }
        it++;
    }
    return true;
}

/**
 *
 * @param lhs left lp3 map
 * @param rhs right lp3 map
 * @return bool is not equal
 * @details
 * The contents of two unordered containers lhs and rhs are equal if the following conditions hold:
 *  - lhs.size() == rhs.size()
 *  - each key, value pair in lhs has the same key, value pair in rhs
 *  The behavior is undefined if Key or T are not EqualityComparable.
 *
 */
template <class K_, class V_, class Hash_, class Pred_, class Allocator_>
bool operator!=(const LP3<K_, V_, Hash_, Pred_, Allocator_>& lhs, const LP3<K_, V_, Hash_, Pred_, Allocator_>& rhs)
{
    return !(rhs == lhs);
}

#if __cplusplus >= 201703L
/**
 *
 * @param lhs one LP3 map
 * @param rhs another LP3 map
 * @details
 * swaps the maps by calling lhs.swap(rhs)
 */
template <class Key, class T, class Hash, class KeyEqual, class Alloc>
void swap(std::unordered_map<Key, T, Hash, KeyEqual, Alloc>& lhs,
          std::unordered_map<Key, T, Hash, KeyEqual, Alloc>& rhs) noexcept
{
    lhs.swap(rhs);
}
#else
/**
 *
 * @param lhs one LP3 map
 * @param rhs another LP3 map
 * @details
 * swaps the maps by calling lhs.swap(rhs)
 */
template <class Key, class T, class Hash, class KeyEqual, class Alloc>
void swap(std::unordered_map<Key, T, Hash, KeyEqual, Alloc>& lhs,
          std::unordered_map<Key, T, Hash, KeyEqual, Alloc>& rhs)
{
    lhs.swap(rhs);
}
#endif

#if __cplusplus >= 202002L

#else
#endif

/**
 * @param c container
 * @param pred  predicate that returns true if the element should be erased
 * @return The number of erased elements.
 */
template <class Key, class T, class Hash, class KeyEqual, class Alloc, class Pred>
typename LP3<Key, T, Hash, KeyEqual, Alloc>::size_t erase_if(LP3<Key, T, Hash, KeyEqual, Alloc>& c, Pred pred)
{
    auto old_size = c.size();
    for (auto i = c.begin(), last = c.end(); i != last;) {
        if (pred(*i)) {
            i = c.erase(i);
        }
        else {
            ++i;
        }
    }
    return old_size - c.size();
}

#endif
