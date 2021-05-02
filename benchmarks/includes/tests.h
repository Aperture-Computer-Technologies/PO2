
#ifndef TESTS_H
#define TESTS_H

#include <algorithm>
#include <chrono>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

// own
#include "./generator.h"
#include "./prepare.h"

using namespace std::chrono;
using std::cout;
using std::vector;

/*
This is yet again a template function.
basic functionality is like this:
1. we create a vector to store the times
2. we create and populate vectors for keys that will be used for the tests
3. create the hashmap.
4. call prepare(hashmap, inserted)
5. populate the hashmap with inserted - 10 k,v pairs
6. benchmark the vector access time, which will be subtracted later
7. insert 10k keys(from insert_keys) and time it
8. lookup 10k keys(from sample_keys) and time it
9. lookup 10k nonexistent keys(nonkeys) and time it
10. delete 10k keys(sample_keys) and time it
times are added to the results vector, and that is returned.

(4) this step is called because some hashmaps require some extra steps before
you use them. For example, setting a key that will be the thombstone marker, the
key that will mark a location as empty, etc.

*/
template <class T>
vector<long int> int_test(int size)
{
    vector<long int> results;  // insert, lookup, unsuccesful lookup, delete times
    vector<int> sample_keys;   // get a sample of keys to lookup and later delete, will be filled later

    // unsuccesful lookup keys
    vector<int> nonkeys(10000);
    // generate uses a function(here, gen_unsuccesfull_int) to fill a container with values
    std::generate(nonkeys.begin(), nonkeys.end(), gen_unsuccesfull_int);

    // keys for insert test
    vector<int> insert_keys(10000);
    std::generate(insert_keys.begin(), insert_keys.end(), gen_int);

    T testmap{};
    prepare(testmap, size);  // do special actions, such as setting the tombstone marker for other, more exotic hashmaps

    {  // seperate scope, so all_keys gets destroyed. for good measure, empty it too
        vector<int> all_keys(size - 10000);
        std::generate(all_keys.begin(), all_keys.end(), gen_int);
        // sample inserts x ammount of values from old_container to
        // new_container with the help of a generator instance
        // in this case, random 10k keys from all_keys to sample_keys
        std::sample(all_keys.begin(), all_keys.end(), std::back_inserter(sample_keys), 10000, generator);

        for (auto i : all_keys) {
            testmap.insert({i, i});
        }
        all_keys.clear();  // going out of scope should call the destructor to
                           // clear it, but just making sure it's done
    }

    // testing vector access times to subtract later
    time_point<steady_clock> vector_start = steady_clock::now();
    for (auto i : sample_keys) {
        if (i == -1) cout << "WTF";  // should never run, is here so loop doesnt get optimized away
    }
    time_point<steady_clock> vector_end = steady_clock::now();
    auto vector_acces_time = duration_cast<nanoseconds>(vector_end - vector_start);

    // insertion test
    // testmap.rehash(testmap.bucket_count()+1); // force a rehash so insertion test has a smaller chance of rehashing
    // during the test
    time_point<steady_clock> insert_start = steady_clock::now();
    for (auto key : insert_keys) {
        testmap.insert({key, key});
    }
    time_point<steady_clock> insert_end = steady_clock::now();

    auto insert_time = (duration_cast<nanoseconds>(insert_end - insert_start) - vector_acces_time) / 10000;
    results.push_back(insert_time.count());
    // clear all values in here, clear up some memory
    insert_keys.clear();

    // lookup test
    time_point<steady_clock> lookup_start = steady_clock::now();
    for (auto key : sample_keys) {
        if (testmap[key] == 0) cout << "WTF";
    }
    time_point<steady_clock> lookup_end = steady_clock::now();
    auto lookup_time = (duration_cast<nanoseconds>(lookup_end - lookup_start) - vector_acces_time) / 10000;
    results.push_back(lookup_time.count());

    // unsuccesful lookup test
    time_point<steady_clock> unlookup_start = steady_clock::now();
    for (auto key : nonkeys) {
        if (testmap[key] == -1) cout << "WTF";
    }
    time_point<steady_clock> unlookup_end = steady_clock::now();
    auto unlookup_time = (duration_cast<nanoseconds>(unlookup_end - unlookup_start) - vector_acces_time) / 10000;
    results.push_back(unlookup_time.count());
    // free some memoru
    nonkeys.clear();

    // delete test
    time_point<steady_clock> delete_start = steady_clock::now();
    for (auto key : sample_keys) {
        testmap.erase(key);
    }

    time_point<steady_clock> delete_end = steady_clock::now();
    auto delete_time = (duration_cast<nanoseconds>(delete_end - delete_start) - vector_acces_time) / 10000;
    results.push_back(delete_time.count());

    //    Iteration time
    time_point<steady_clock> iter_start = steady_clock::now();
    auto iter = testmap.begin();
    while (++iter != testmap.end()) {
        if (iter->second == -1) cout << "WTF";
    }
    time_point<steady_clock> iter_end = steady_clock::now();
    auto iter_time = (duration_cast<nanoseconds>(iter_end - iter_start)) / testmap.size();
    results.push_back(iter_time.count());
    testmap.clear();
    return results;
}

// pretty much the same, but with strings
// the reason it's split up in 2 functions is because we need other functions to
// generate the keys, and unfortunately we can't overload based on return type
template <class T>
vector<long int> string_test(int size)
{
    vector<long int> results;    // insert, lookup, unsuccesful lookup, delete times
    vector<string> sample_keys;  // get a sample of keys to lookup and later delete

    // unsuccesful lookup keys
    vector<string> nonkeys(10000);
    std::generate(nonkeys.begin(), nonkeys.end(), gen_unsuccesfull_string);

    // keys for insert test
    vector<string> insert_keys(10000);
    std::generate(insert_keys.begin(), insert_keys.end(), gen_string);

    T testmap{};
    prepare(testmap, size);  // do special actions, such as setting the tombstone marker for other, more exotic hashmaps

    {  // seperate scope, so all_keys gets destroyed. for good measure, empty it too
        vector<string> all_keys(size - 10000);
        std::generate(all_keys.begin(), all_keys.end(), gen_string);
        std::sample(all_keys.begin(), all_keys.end(), std::back_inserter(sample_keys), 10000, generator);

        for (const auto& i : all_keys) {
            testmap.insert({i, i});
        }
        all_keys.clear();
    }

    // testing vector access times to subtract later
    time_point<steady_clock> vector_start = steady_clock::now();
    for (auto& i : sample_keys) {
        if (i == "") cout << "WTF";  // should never run, is here so loop doesnt get optimized away
    }
    time_point<steady_clock> vector_end = steady_clock::now();
    auto vector_acces_time = duration_cast<nanoseconds>(vector_end - vector_start);

    // insertion test
    time_point<steady_clock> insert_start = steady_clock::now();
    for (auto key : insert_keys) {
        testmap.insert({key, key});
    }
    time_point<steady_clock> insert_end = steady_clock::now();

    auto insert_time = (duration_cast<nanoseconds>(insert_end - insert_start) - vector_acces_time) / 10000;
    results.push_back(insert_time.count());
    // remove some memory
    insert_keys.clear();

    // lookup test
    time_point<steady_clock> lookup_start = steady_clock::now();
    for (auto key : sample_keys) {
        if (testmap[key] == "") cout << "WTF";
    }
    time_point<steady_clock> lookup_end = steady_clock::now();
    auto lookup_time = (duration_cast<nanoseconds>(lookup_end - lookup_start) - vector_acces_time) / 10000;
    results.push_back(lookup_time.count());

    // unsuccesful lookup test
    time_point<steady_clock> unlookup_start = steady_clock::now();
    for (auto key : nonkeys) {
        if (testmap[key] == "a") cout << "WTF";
    }
    time_point<steady_clock> unlookup_end = steady_clock::now();
    auto unlookup_time = (duration_cast<nanoseconds>(unlookup_end - unlookup_start) - vector_acces_time) / 10000;
    results.push_back(unlookup_time.count());
    // free some memoru
    nonkeys.clear();

    // delete test
    time_point<steady_clock> delete_start = steady_clock::now();
    for (auto key : sample_keys) {
        testmap.erase(key);
    }

    time_point<steady_clock> delete_end = steady_clock::now();
    auto delete_time = (duration_cast<nanoseconds>(delete_end - delete_start) - vector_acces_time) / 10000;
    results.push_back(delete_time.count());

    //    Iteration time
    time_point<steady_clock> iter_start = steady_clock::now();
    auto iter = testmap.begin();
    while (++iter != testmap.end()) {
        if (iter->second == "a") cout << "WTF";
    }
    time_point<steady_clock> iter_end = steady_clock::now();
    auto iter_time = (duration_cast<nanoseconds>(iter_end - iter_start)) / testmap.size();
    results.push_back(iter_time.count());

    testmap.clear();
    return results;
}

#endif /* TESTS_H */