#ifndef LP33_H
#define LP33_H

#include <algorithm>
#import <cassert>
#include <functional>
#include <iterator>
#include <numeric>
#include <random>
#include <vector>

#include "fastmod.h"
#include "plf_colony.h"

namespace LP {

    /*
     * Randomness generator.
     */
    static std::mt19937 gener(INT32_MAX - 2020);
    /*
     * this converts the random data and distributes it over [1, int32 max]
     */
    static std::uniform_int_distribution<int> random_int_distr(1, INT32_MAX);
    // gen integers between 0 and int32max
    inline int gen_integer() { return random_int_distr(gener); }

    /*
     * prime sizes.
     * the next primesize that's 2x the last one till 100m, then 1.25x the last prime size
     */
    static constexpr size_t prime_sizes[]
        = {127,        251,        479,        911,        1733,      3299,      6269,       11923,      22669,
           43093,      81883,      155579,     295601,     561667,    1067179,   2027659,    3852553,    7319857,
           13907737,   26424707,   50206957,   95393219,   143731457, 179424989, 224367413,  280465301,  350916677,
           373588249,  467886691,  573259913,  717266647,  776531999, 971057303, 1190495191, 1400305763, 1611624473,
           1824261979, 2252945627, 2685457727, 3340200581, 4000846897};

    inline size_t next_prime(const size_t& n)
    {
        size_t next = n;
        for (const int x : prime_sizes) {
            if (x > n) {
                next = x;
                break;
            }
        }
        return next;
    }

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

    /**
     * @brief Alternative to plf::colony::iterator
     * @details
     * This is a way to store the plf::colony::iterator.
     * This is done for 2 reasons:
     * 1. remove a static cast from plf::colony<Pair_elem>::aligned_pointer_type to Pair_elem
     * 2. I wanted to replace the skipfield and grouptype pointers with 4byte ints instead of 8byte pointers
     * making the bucket interface smaller, but making deletes slightly more expensive because of reconstruction.
     * I __might__ do that some day.
     * The problem is that I need to remove 11 bytes total from Bucket_wrapper to add 1 more bucket/cache line,
     * and I can at most remove 4 bytes with this. group type isn't always within 32bit max of elem_type.
     * I'd have to shave off some bits from the hash in the bucket interface instead.
     */
    template <typename K, typename V, class Allocator = std::allocator<std::pair<const K, V>>>
    class naive_faster_colony_iter {  // saving a reinterpret cast, giving me  a 20ns speedup
        using Pair_elem = std::pair<const K, V>;
        using iter = typename plf::colony<Pair_elem, Allocator>::iterator;
        using plf_constiter = typename plf::colony<Pair_elem, Allocator>::const_iterator;
        using group_type = typename plf::colony<Pair_elem>::group_pointer_type;
        using skipfield_type = typename plf::colony<Pair_elem>::skipfield_pointer_type;
        using elem_type = typename plf::colony<Pair_elem>::aligned_pointer_type;
        Pair_elem* elem;
        group_type group;
        skipfield_type skipfield;

      public:
        /**
         * @brief conversion constructor
         * @param iter plf::colony::iterator
         */
        naive_faster_colony_iter(iter& iter)
            : elem{reinterpret_cast<Pair_elem*>(iter.element_pointer)},
              group{iter.group_pointer},
              skipfield{iter.skipfield_pointer} {};
        /**
         * @brief default constructor
         */
        inline naive_faster_colony_iter() : elem{nullptr}, group{nullptr}, skipfield{0} {};
        /**
         * @return plf::colony::iterator
         */
        inline iter convert() const { return iter(group, reinterpret_cast<elem_type>(elem), skipfield); }
        //        inline iter convertconst() const { return iter(group, reinterpret_cast<elem_type>(elem), skipfield); }

        /**
         * pointer to member
         * @return
         */
        Pair_elem* operator->() const { return elem; }
    };
    constexpr int32_t DELETED = -1;
    constexpr int32_t EMPTY = -2;
    /**
     *  @brief  wrapper for storing hashes and std::pair<K, V>*
     *  @tparam  K Key
     *  @tparam V Value
     *  @details
     * This is a struct I use instead of std::pair<int, colony iterator>
     * It's just here for added semantic value, plus having a guaranteed data layout I can guarantee
     * across compilers.
     * @todo try shaving off bytes. need to go from 32 to 21 bytes/bucket to add 1 to the cache line
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
        Bucket_wrapper() : hash{LP::EMPTY}, pair_iter{} {};  //
        /**
         * @brief Copy constructor
         * @param e other Bucket_wrapper
         */
        Bucket_wrapper(const Bucket_wrapper& e) : hash{e.hash}, pair_iter{e.pair_iter} {};
        int32_t hash;
        iter pair_iter;
    };

}  // namespace LP

/**
 * @brief Linear probing map
 * @tparam K Key
 * @tparam V Value
 * @tparam Hash Hashing function that should be used to hash keys
 * @tparam Pred equality function to check if keys are equal
 * @tparam Allocator=std::allocator the allocator
 *
 * @details
 * A linear probing map
 * https://www.plflib.org/colony.htm#faq
 * The highlights of colony include:
 * 1. can insert without reference AND iter invalidation
 * 2. can delete without reference and iter invalidation
 * 3. performance on par with deque
 *
 * It has all the good bits of deque without the downsides.
 * Using it does bring some additional downsides, however.
 * To be able to delete, I need to store an iter to it's position.
 * a Colony::iter is bigger than a pointer (32 bytes vs 16 bytes)
 * I can only fit 2 buckets in a cache line instead of the previous 4.
 * Consequently, probing is slower. But now, everything works.
 * potential solutions:
 * 1. shave off 7 bytes of the bucket
 * Do that by converting the pointers to custom length integers. Try to find a pattern in the iterator internals.
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
    using Bucket = LP::Bucket_wrapper<K, V>;
    using Pair_elem = std::pair<const K, V>;
    using plf_iter = typename plf::colony<Pair_elem, Allocator>::iterator;
    using plf_constiter = typename plf::colony<Pair_elem, Allocator>::const_iterator;

  private:
    Hash user_hash;
    Pred is_equal;
    int inserted_n;
    uint64_t modulo_help;               // faster modulo trick thing, see lemire's fastmod
    float lf_max;                       // max loadfactor
    std::vector<Bucket> hash_store;     // stores <hash, kv_pair iterator>
    std::vector<int32_t> random_state;  // random bits used for hashing
    plf::colony<Pair_elem, Allocator> kv_store;

    int32_t hasher(const K& key) const;                       // hashes key
    void hasher_state_gen();                                  // generates randomness for hashing function
    size_t prober(const K& key, const int32_t& hash) const;  // probes where key should be at
    void rehash(size_t size);                                 // rehashes
    LP::Result contains_key(const K& key) const;              // prober() with extended info
    void rehash_if_needed();

  public:
    // iterators

    struct ConstIterator;  // iterator needs it to declar friend
    struct Iterator {
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

    struct ConstIterator {
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
        friend bool operator==(const Iterator& a, const ConstIterator& b) { return a.slave == b.slave; };
        friend bool operator!=(const Iterator& a, const ConstIterator& b) { return a.slave != b.slave; };
        friend bool operator==(const ConstIterator& a, const Iterator& b) { return a.slave == b.slave; };
        friend bool operator!=(const ConstIterator& a, const Iterator& b) { return a.slave != b.slave; };
    };

    Iterator begin() { return Iterator(kv_store.begin()); };
    Iterator end() { return Iterator(kv_store.end()); };
    ConstIterator cbegin() const { return ConstIterator(kv_store.cbegin()); };
    ConstIterator cend() const { return ConstIterator(kv_store.cend()); };

    // some required typedefs
    using value_type = Pair_elem;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = Iterator;
    using const_iterator = ConstIterator;
    using difference_type = typename iterator::difference_type;
    using size_type = difference_type;

    // constructors and destructors
    LP3();
    explicit LP3(size_t size, const Hash& hash = Hash(), const Pred& equal = Pred(),
                 const Allocator& alloc = Allocator());
    LP3(size_t size, const Allocator& alloc);
    LP3(size_t size, const Hash& hash, const Allocator& alloc);
    explicit LP3(const Allocator& alloc);
    template <class InputIt>
    LP3(InputIt first, InputIt last);
    template <class InputIt>
    LP3(InputIt first, InputIt last, size_t size);
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
    // insertion overloads.
    std::pair<Iterator, bool> insert(const Pair_elem& value);  // copy
    Iterator insert(ConstIterator hint, const Pair_elem& kv);  // copy
    template <class P>
    std::pair<Iterator, bool> insert(P&& value);  // delegate to move insert
    template <class P>
    Iterator insert(ConstIterator hint, P&& value);
    /* TODO: the above are meant to eliminate a move/copy. instead of convert, copy/move, it should do convert only
     * atm, they don't do that, but they do correctly insert
     */
    template <class InputIt>
    void insert(InputIt first, InputIt last);                      // delegate
    void insert(std::initializer_list<Pair_elem> ilist);           // delegate
#if __cplusplus >= 201703L                                         // if C++17+
    std::pair<Iterator, bool> insert(Pair_elem&& value);           // move
    Iterator insert(ConstIterator hint, const Pair_elem&& value);  // move, delegate to above
    //    insert or assign overloads
    template <class M>
    std::pair<Iterator, bool> insert_or_assign(const K& k, M&& obj);  // copy key
    template <class M>
    std::pair<Iterator, bool> insert_or_assign(K&& k, M&& obj);  // move key
    template <class M>
    Iterator insert_or_assign(ConstIterator hint, const K& k, M&& obj);
    template <class M>
    Iterator insert_or_assign(ConstIterator hint, K&& k, M&& obj);
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

    size_t erase(const K& key);
    Iterator erase(ConstIterator pos);
    Iterator erase(ConstIterator first, ConstIterator last);
#if __cplusplus >= 201703L
    Iterator erase(Iterator pos);
#else
#endif
    //   modifying lookups
    V& operator[](const K& k);
    V& operator[](K&& k);
    // nonmodifying lookups
    V& at(const K& k);
    const V& at(const K& k) const;
    size_t count(const K& key) const;
    Iterator find(const K& key);
    ConstIterator find(const K& key) const;
//    template< class K > iterator find( const K& x ); //since C++20) // TODO: someday
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
    size_t bucket_count() const { return hash_store.size(); };
    size_t max_bucket_count() const { return max_size(); };
    size_t bucket_size(size_t n) const { return (kv_store[n].hash >= 0); };
    size_t bucket(const K& key) const { return contains_key(key).pos; };
    //    Hash policy
    float load_factor() const { return kv_store.size() / (float)hash_store.size(); };
    float max_load_factor() const { return lf_max; };
    void max_load_factor(float ml);
    void rehash();
    void reserve(int size);

    //     observers
    Hash hash_function() const { return Hash{}; };
    Pred key_eq() const { return Pred{}; };
};

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
bool operator==(const LP3<K_, V_, Hash_, Pred_, Allocator_>& lhs, const LP3<K_, V_, Hash_, Pred_, Allocator_>& rhs);

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
bool operator!=(const LP3<K_, V_, Hash_, Pred_, Allocator_>& lhs, const LP3<K_, V_, Hash_, Pred_, Allocator_>& rhs);

#if __cplusplus >= 201703L
/**
 *
 * @param lhs one LP3 map
 * @param rhs another LP3 map
 * @details
 * swaps the maps by calling lhs.swap(rhs)
 */
template <class Key, class T, class Hash, class KeyEqual, class Alloc>
void swap(LP3<Key, T, Hash, KeyEqual, Alloc>& lhs, LP3<Key, T, Hash, KeyEqual, Alloc>& rhs) noexcept;
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
          std::unordered_map<Key, T, Hash, KeyEqual, Alloc>& rhs);
#endif

#ifndef LP3_DEF_H

// helper futions defs

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
    std::generate(state.begin(), state.end(), LP::gen_integer);
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
size_t LP3<K, V, Hash, Pred, Allocator>::prober(const K& key, const int32_t& hash) const
{
    unsigned long size = hash_store.size();
    size_t pos = fastmod::fastmod_s32(hash, modulo_help, size);
        for (int i = 0; i < size; i++) {
            if (hash_store[pos].hash != LP::EMPTY
                && (hash_store[pos].hash != hash || not is_equal(hash_store[pos].pair_iter->first, key))) {
                pos++;
                if (pos >= size) {
                    pos -= size;
                }
            }
            else {
                return pos;
            }
        }
//        return pos;
}

/**
 * @brief checks if key exists, for internal use only
 * @param key
 * @return Result{exists, position, hash}
 * @details
 * probe bucket arr, and if the resulting position is empty, key doesn't exist
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
LP::Result LP3<K, V, Hash, Pred, Allocator>::contains_key(const K& key) const
{
    int32_t hash = hasher(key);
    int pos = prober(key, hash);

    if (hash_store[pos].hash == LP::EMPTY) {
        return {false, pos, hash};
    }
    return {true, pos, hash};
}

// // Removing 2 lines of code I have to write in every insert function
// template <typename K, typename V, typename Hash, typename Pred, class Allocator>
// void LP3<K, V, Hash, Pred, Allocator>::rehash_if_needed()
//{
//    if (((inserted_n + 1) / (float)hash_store.size()) > lf_max) {
//        rehash();
//    }
//}
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
 * reason why i'm not doing only LP3(size=something) is compiler complaints
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
LP3<K, V, Hash, Pred, Allocator>::LP3(size_t size, const Hash& hash, const Pred& equal, const Allocator& alloc)
    : is_equal(Pred()),
      inserted_n{0},
      modulo_help(fastmod::computeM_s32(LP::next_prime(2 * size))),
      lf_max{0.5},
      hash_store{std::vector<Bucket>(LP::next_prime(2 * size))},
      kv_store{}
{
    hasher_state_gen();
}

/**
 *
 * @param bucket_count the bucket count
 * @param alloc the allocator
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
LP3<K, V, Hash, Pred, Allocator>::LP3(size_t bucket_count, const Allocator& alloc) : LP3{bucket_count}
{
}
/**
 *@details Constructor with user-supplied size, hash and allocator
 * @param size
 * @param hash hash function
 * @param alloc allocator
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
LP3<K, V, Hash, Pred, Allocator>::LP3(size_t size, const Hash& hash, const Allocator& alloc) : LP3{size}
{
    user_hash = hash;
}

/**
 * @brief constructor with euser supplied allocator
 * @param size
 * @param hash hash function
 * @param alloc allocator
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
LP3<K, V, Hash, Pred, Allocator>::LP3(const Allocator& alloc) : LP3{}
{
}

/**
 * @brief Constructs LP3 from iterators
 * @param first iterator to first element you want to include
 * @param last iterator to last element of the range you want to include
 * @details
 * Constraint: `std::is_constructible<std::pair<const K,V>, typename
 * std::iterator_traits<InputIt>::value_type>::value` is true
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
template <class InputIt>
LP3<K, V, Hash, Pred, Allocator>::LP3(InputIt first, InputIt last) : LP3{LP3<K, V>(std::distance(first, last))}
{
    static_assert(std::is_constructible<Pair_elem, typename std::iterator_traits<InputIt>::value_type>::value,
                  "Iterator's value_type must be able to construct a pair<const K, V>");
    while (first != last) {
        insert(*first++);
    }
}

/**
 * @brief Constructor from iterator pair with type hint
 * @param first iterator to first element you want to include
 * @param last iterator to last element of the range you want to include
 * @param size size suggestion. May be ignored.
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
template <class InputIt>
LP3<K, V, Hash, Pred, Allocator>::LP3(InputIt first, InputIt last, size_t size)
    : LP3{LP3<K, V>(std::max((size_t)std::distance(first, last), (size_t)size))}
{
    static_assert(std::is_constructible<Pair_elem, typename std::iterator_traits<InputIt>::value_type>::value,
                  "Iterator's value_type must be able to construct a pair<const K, V>");
    while (first != last) {
        insert(*first++);
    }
}

/**
 * @details Copy constructor
 * @param other Other LP3 you want to copy
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
LP3<K, V, Hash, Pred, Allocator>::LP3(const LP3& other)
    : is_equal(other.is_equal),
      inserted_n{other.inserted_n},
      modulo_help(other.modulo_help),
      lf_max{other.lf_max},
      hash_store{other.hash_store.size()},
      random_state{other.random_state},
      kv_store{other.kv_store}
{
    for (auto it = kv_store.begin(); it != kv_store.end(); it++) {
        auto pos_info = contains_key(it->first);
        hash_store[pos_info.pos] = Bucket{pos_info.hash, it};
    }
};

/**
 * @details Move constructor
 * @param other Other hashmap you want to move
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
LP3<K, V, Hash, Pred, Allocator>::LP3(LP3&& other)
{
    other.swap(*this);
    other.clear();
};

/**
 * @brief constructor from initializer list
 * @param init initializer_list
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
LP3<K, V, Hash, Pred, Allocator>::LP3(std::initializer_list<Pair_elem> init) : LP3{LP3<K, V>{init.size()}}
{
    for (const auto& x : init) {
        insert(x);
    }
}
/**
 * @brief constructor from initializer list
 * @param init initializer_list
 * @param bucket_count bucket count. It may be ignored
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
LP3<K, V, Hash, Pred, Allocator>::LP3(std::initializer_list<Pair_elem> init, size_t bucket_count)
    : LP3{LP3<K, V>(std::max(bucket_count, init.size()))}
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
    std::swap(*this, temp);
    for (auto it = kv_store.begin(); it != kv_store.end(); it++) {
        auto pos_info = contains_key(it->first);
        hash_store[pos_info.pos] = Bucket{pos_info.hash, it};
    }
    return *this;
}

#    if __cplusplus >= 201703L
/**
 *
 * @param other map you want to move assign
 * @return this with the new state
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
LP3<K, V, Hash, Pred, Allocator>& LP3<K, V, Hash, Pred, Allocator>::operator=(LP3&& other) noexcept
{
    swap(other);
    return *this;
}
#    else
/**
 * @param other map you want to move assign
 * @return this with the new state
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
LP3<K, V, Hash, Pred, Allocator>& LP3<K, V, Hash, Pred, Allocator>::operator=(LP3&& other)
{
    swap(*this, other);
    return *this;
}
#    endif
/**
 * @brief copy assign from initializer list
 * @param ilist initializer list
 * @return
 */
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
 * @param kv std::pair<K,V> lvalue reference that needs to be inserted
 * @brief inserts kv if kv.first doesn't exist in map
 * @details
 * inserts element, returns pair<iterator to map[k], bool is inserted>
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

#    if __cplusplus >= 201703L  // if C++17+
/**
 *
 * @param kv std::pair<K,V> rvalue reference that needs to be inserted
 * @brief inserts kv if kv.first doesn't exist in map
 * @details
 * inserts element, returns pair<iterator to map[k], bool is inserted>
 */
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
    auto it = kv_store.insert(std::forward<Pair_elem>(kv));
    hash_store[pos_info.pos] = Bucket{pos_info.hash, it};
    inserted_n++;
    return std::pair<Iterator, bool>(it, true);
}

/**
 * @param hint iterator where it should be inserted to. It may be ignored.
 * @param kv std::pair<K,V> lvalue reference that needs to be inserted
 * @brief inserts kv if kv.first doesn't exist in map
 * @details
 * inserts element, returns iterator to map[k]
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
typename LP3<K, V, Hash, Pred, Allocator>::Iterator LP3<K, V, Hash, Pred, Allocator>::insert(ConstIterator hint,
                                                                                             const Pair_elem&& kv)
{
    if (((inserted_n + 1) / (float)hash_store.size()) > lf_max) {
        rehash();
    }
    auto pos_info = contains_key(kv.first);
    if (pos_info.contains) {
        return hash_store[pos_info.pos].pair_iter.convert();
    }
    auto it = kv_store.insert(std::move(kv));
    hash_store[pos_info.pos] = Bucket{pos_info.hash, it};
    inserted_n++;
    return it;
}

#    else
#    endif
/**
 * @param hint iterator where it should be inserted to. It may be ignored.
 * @param kv std::pair<K,V> lvalue reference that needs to be inserted
 * @brief inserts kv if kv.first doesn't exist in map
 * @details
 * inserts element, returns iterator to map[k]
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
typename LP3<K, V, Hash, Pred, Allocator>::Iterator LP3<K, V, Hash, Pred, Allocator>::insert(ConstIterator hint,
                                                                                             const Pair_elem& kv)
{
    return insert(kv).first;
}

/**
 * @detail insertion with P value as argument
 * std::is_constructible<std::pair<const K,V>, P>::value must be true
 * @tparam P type for which std::is_constructible<Pair_elem, std::pair<Args...>>::value is true
 * @param value element to insert
 * @return pair<iterator, bool is inserted>
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
template <class P>
std::pair<typename LP3<K, V, Hash, Pred, Allocator>::Iterator, bool> LP3<K, V, Hash, Pred, Allocator>::insert(P&& value)
{
    static_assert(std::is_constructible<Pair_elem, P>::value);
    return insert(std::forward<Pair_elem>(Pair_elem{value}));
}

/**
 * @detail insertion with P value as argument and insertion hint (that's ignored)
 * std::is_constructible<std::pair<const K,V>, P>::value must be true
 * @tparam P type for which std::is_constructible<Pair_elem, std::pair<Args...>>::value is true
 * @param value element to insert
 * @return iterator to inserted or existing element
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
template <class P>
typename LP3<K, V, Hash, Pred, Allocator>::Iterator LP3<K, V, Hash, Pred, Allocator>::insert(ConstIterator hint,
                                                                                             P&& value)
{
    static_assert(std::is_constructible<Pair_elem, P>::value);
    if (((inserted_n + 1) / (float)hash_store.size()) > lf_max) {
        rehash();
    }
    Pair_elem kv{value};
    auto pos_info = contains_key(kv.first);
    if (pos_info.contains) {
        return hash_store[pos_info.pos].pair_iter.convert();
    }
    auto it = kv_store.insert(std::move(kv));
    hash_store[pos_info.pos] = Bucket{pos_info.hash, it};
    inserted_n++;
    return it;
}
/**
 *
 * @tparam InputIt iterator where std::is_constructible<Pair_elem, typename
 * std::iterator_traits<InputIt>::value_type>::value is true
 * @param first iterator to first element of the iter range that needs to be inserted
 * @param last iterator to last element of the iter range that needs to be inserted
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
template <class InputIt>
void LP3<K, V, Hash, Pred, Allocator>::insert(InputIt first, InputIt last)
{
    while (first != last) {
        insert(*first++);
    }
}

/**
 * @param ilist initilizer list of kv pairs
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
void LP3<K, V, Hash, Pred, Allocator>::insert(std::initializer_list<Pair_elem> ilist)
{
    for (auto x : ilist) {
        insert(std::move(x));
    }
}

// --------------------- end insert overloads

// --------------------- begin insert_or_assign_overloads

#    if __cplusplus >= 201703L
/**
 *

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
        inserted_n++;
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
        return {it.convert(), false};
    }
    else {
        auto it = kv_store.insert({std::forward<K>(k), std::forward<M>(obj)});
        hash_store[pos_info.pos] = {pos_info.hash, it};
        inserted_n++;
        return {it, true};
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
typename LP3<K, V, Hash, Pred, Allocator>::Iterator LP3<K, V, Hash, Pred, Allocator>::insert_or_assign(
    ConstIterator hint, const K& k, M&& obj)
{
    return insert_or_assign(k, std::forward<M>(obj)).first;
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
    return insert_or_assign(std::forward<K>(k), std::forward<M>(obj)).first;
}
#    else
#    endif

// -------------------- end insert or assign overloads

/**
 * @details
 * Inserts a new element into the container constructed in-place with the given args if there is no element with the
 * key in the container. The element may be constructed even if there already is an element with the key in the
 * container, in which case the newly constructed element will be destroyed immediately. Currently, it IS always
 * constructed regardless if it exists.
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
    return insert(std::forward<Pair_elem>(temp));
}

/**
 * @details emplace with iterator hint. hint may be ignored.
 * @param hint
 * @param args
 * @return
 */
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
    return insert(std::forward<Pair_elem>(temp)).first;
}

/**
 * @details swap 2 hashmaps with each other
 */
#    if __cplusplus >= 201703L
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
#    else
/**
 * @brief swaps LP3 instances
 */
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
#    endif

/**
 * @brief erase elements..
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
size_t LP3<K, V, Hash, Pred, Allocator>::erase(const K& key)
{
    auto pos_info = contains_key(key);
    if (not pos_info.contains) {
        return 0;
    }
    auto pos = pos_info.pos;
    hash_store[pos].hash = LP::DELETED;
    kv_store.erase(hash_store[pos].pair_iter.convert());
    inserted_n--;
    return 1;
}

/**
 * @param it iterator to element that will be deleted
 * @return it++
 * @details
 * if it == LP3.cend(), returns cend()
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
typename LP3<K, V, Hash, Pred, Allocator>::Iterator LP3<K, V, Hash, Pred, Allocator>::erase(ConstIterator it)
{
    if (it == kv_store.cend()) {
        return Iterator{it.slave};
    }
    auto pos_info = contains_key(it->first);
    auto pos = pos_info.pos;
    hash_store[pos].hash = LP::DELETED;
    kv_store.erase(it.slave);
    inserted_n--;
    return Iterator{++it.slave};
}

/**
 * @param first, last range of elements to delete
 * @return last++
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
typename LP3<K, V, Hash, Pred, Allocator>::Iterator LP3<K, V, Hash, Pred, Allocator>::erase(ConstIterator first,
                                                                                            ConstIterator last)
{
    while (first != last) {
        erase(first++);
    }
    return Iterator(last.slave);
}

#    if __cplusplus >= 201703L
/**
 * @param it iterator to element that will be deleted
 * @return it++
 * @details
 * if it == LP3.cend(), returns cend()
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
typename LP3<K, V, Hash, Pred, Allocator>::Iterator LP3<K, V, Hash, Pred, Allocator>::erase(Iterator it)
{
    return erase(ConstIterator{it});
}
#    else
#    endif

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
    auto it = kv_store.insert(Pair_elem{k, V{}});  // change back to forward later
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
        return hash_store[pos_info.pos].pair_iter->second;
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
        return hash_store[pos_info.pos].pair_iter->second;
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
        Iterator it = hash_store[pos_info.pos].pair_iter.convert();
        return {it};
    }
    return kv_store.cend();
}

#    if __cplusplus >= 202002L
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

    if (hash_store[pos].hash == LP::EMPTY) {
        return false;
    }
    return true;
}
#    else
#    endif

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
    if (kv_store.size() / (float)hash_store.size() > ml) {
        rehash(inserted_n / ml);
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
 * @bug it actually doesn't respect loadfactor_max, so it will definitely rehash if you try to insert n=size
 * elements
 */
template <typename K, typename V, typename Hash, typename Pred, class Allocator>
void LP3<K, V, Hash, Pred, Allocator>::rehash(size_t size)
{
    std::vector<Bucket> arr_new(size);
    uint64_t helper = fastmod::computeM_s32(size);
    for (const auto& x : hash_store) {
        if (x.hash < 0) {
            continue;
        }
        //        int32_t loc = x.hash % size;
        int32_t loc = fastmod::fastmod_s32(x.hash, helper, size);
        while (arr_new[loc].hash != LP::EMPTY) {
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
    int size = LP::next_prime(int(kv_store.size() / lf_max));
    rehash(size);
}

/**
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

/**
 * @param c container
 * @param pred  predicate that returns true if the element should be erased
 * @return The number of erased elements.
 */
template <class Key, class T, class Hash, class KeyEqual, class Alloc, class Pred>
size_t erase_if(LP3<Key, T, Hash, KeyEqual, Alloc>& c, Pred pred)
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
    return not(lhs == rhs);
}

#    if __cplusplus >= 201703L
/**
 *
 * @param lhs one LP3 map
 * @param rhs another LP3 map
 * @details
 * swaps the maps by calling lhs.swap(rhs)
 */
template <class Key, class T, class Hash, class KeyEqual, class Alloc>
void swap(LP3<Key, T, Hash, KeyEqual, Alloc>& lhs, LP3<Key, T, Hash, KeyEqual, Alloc>& rhs) noexcept
{
    lhs.swap(rhs);
}
#    else
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
#    endif

#    if __cplusplus >= 202002L

#    else
#    endif

#endif  // LP3_DEF_H
#endif  // LP3_H
