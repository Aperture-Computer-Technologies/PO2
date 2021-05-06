//
// Created by MassiveAtoms on 4/29/21.
//
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch.hpp>
// Seperate this from the source file containing tests
// removing the need to recompile catch again and again

#include <iostream>
#include <set>
#include <unordered_map>

#include "./../hashmap_implementations/LPmap3.h"

using Pair_elem = std::pair<const int, int>;
using Pair_float = std::pair<const float, float>;
using std::cout;

//Tests that are done
TEMPLATE_TEST_CASE("equality operators work", "[equality]", (std::unordered_map<int, int>), (LP3<int, int>))
{
    // fill a hashmap with 1000 values, for constructors that copy/move or otherwise construct from another hashmap
    TestType copymap;
    for (int i = 0; i < 1000; i++) {
        copymap.insert({i, i + 1});
    }
    SECTION("EQUALITY")
    {
        TestType testmap{copymap};
        TestType copy2{copymap};
        TestType copy3{copymap};
        copy2.insert({8000, 7});
        copy3[5] = 999;
        REQUIRE((copymap == testmap) == true);
        REQUIRE((copy2 == testmap) == false);
        REQUIRE((copy3 == testmap) == false);
        REQUIRE((copymap != testmap) != (copymap == testmap));
        REQUIRE((copy2 != testmap) != (copy2 == testmap));
        REQUIRE((copy3 != testmap) != (copy3 == testmap));
    }
}

TEMPLATE_TEST_CASE("constructors work", "[constructors]", (std::unordered_map<int, int>), (LP3<int, int>))
{
    // fill a hashmap with 1000 values, for constructors that copy/move or otherwise construct from another hashmap
    TestType copymap;
    for (int i = 0; i < 1000; i++) {
        copymap.insert({i, i + 1});
    }

    SECTION("Default constructor")
    {
        TestType testmap{};
        REQUIRE(testmap.bucket_count() > 0);
        REQUIRE(testmap.size() == 0);
        REQUIRE(testmap.empty() == true);
        bool iter_empty = (testmap.begin() == testmap.end() && testmap.cbegin() == testmap.cend());
        REQUIRE(iter_empty);
    }
    SECTION("Specify size")
    {
        TestType testmap{250};
        REQUIRE(testmap.bucket_count() > 250);
        REQUIRE(testmap.size() == 0);
        REQUIRE(testmap.empty() == true);
        TestType map2{};
        map2.reserve(250);
        REQUIRE(sizeof(testmap) == sizeof(map2));
        bool iter_empty = (testmap.begin() == testmap.end() && testmap.cbegin() == testmap.cend());
        REQUIRE(iter_empty);
    }
    SECTION("Specify size and allocator")
    {
        TestType testmap{250, std::allocator<Pair_elem>()};
        REQUIRE(testmap.get_allocator() == std::allocator<Pair_elem>());
        REQUIRE(testmap.bucket_count() > 250);
        REQUIRE(testmap.size() == 0);
        REQUIRE(testmap.empty() == true);
        TestType map2{};
        map2.reserve(250);
        REQUIRE(sizeof(testmap) == sizeof(map2));
        bool iter_empty = (testmap.begin() == testmap.end() && testmap.cbegin() == testmap.cend());
        REQUIRE(iter_empty);
    }

    SECTION("Specify size, hash and allocator")
    {
        auto hasher = std::hash<int>{};
        TestType testmap{250, hasher, std::allocator<Pair_elem>()};
        REQUIRE(testmap.get_allocator() == std::allocator<Pair_elem>());
        REQUIRE(testmap.bucket_count() > 250);
        REQUIRE(testmap.size() == 0);
        REQUIRE(testmap.empty() == true);
        TestType map2{};
        map2.reserve(250);
        REQUIRE(sizeof(testmap) == sizeof(map2));
        bool iter_empty = (testmap.begin() == testmap.end() && testmap.cbegin() == testmap.cend());
        REQUIRE(iter_empty);
    }

    SECTION("Specify size and allocator")
    {
        TestType testmap{250, std::allocator<Pair_elem>()};
        REQUIRE(testmap.get_allocator() == std::allocator<Pair_elem>());
        REQUIRE(testmap.bucket_count() > 250);
        REQUIRE(testmap.size() == 0);
        REQUIRE(testmap.empty() == true);
        TestType map2{};
        map2.reserve(250);
        REQUIRE(sizeof(testmap) == sizeof(map2));
        bool iter_empty = (testmap.begin() == testmap.end() && testmap.cbegin() == testmap.cend());
        REQUIRE(iter_empty);
    }

    SECTION("copy with testtype iterators")
    {
        TestType testmap{copymap.begin(), copymap.end()};
        REQUIRE(testmap.bucket_count() == copymap.bucket_count());
        REQUIRE(testmap.size() == copymap.size());
        REQUIRE(testmap.empty() == false);
        bool passed = true;
        for (int i = 0; i < 1000; i++) {
            if (testmap[i] != i + 1) {
                passed = false;
                break;
            }
        }
        REQUIRE(passed);
    }

    SECTION("copy with unordered iterators")
    {
        std::unordered_map<int, int> copy2{copymap.begin(), copymap.end()};
        TestType testmap{copy2.begin(), copy2.end()};
        REQUIRE(testmap.bucket_count() == copymap.bucket_count());
        REQUIRE(testmap.size() == copymap.size());
        REQUIRE(testmap.empty() == false);
        bool passed = true;
        for (int i = 0; i < 1000; i++) {
            if (testmap[i] != i + 1) {
                passed = false;
                break;
            }
        }
        REQUIRE(passed);
    }

    SECTION("copy with iterators with specified correct size")
    {
        std::unordered_map<int, int> copy2{copymap.begin(), copymap.end()};
        TestType testmap{copy2.begin(), copy2.end(), 1000};
        REQUIRE(testmap.size() == copy2.size());
        REQUIRE(testmap.empty() == false);
        bool passed = true;
        for (int i = 0; i < 1000; i++) {
            if (testmap[i] != i + 1) {
                passed = false;
                break;
            }
        }
        REQUIRE(passed);
    }

    SECTION("copy with iterators with specified smaller size")
    {
        std::unordered_map<int, int> copy2{copymap.begin(), copymap.end()};
        TestType testmap{copy2.begin(), copy2.end(), 10};
        REQUIRE(testmap.bucket_count() >= copymap.bucket_count());
        REQUIRE(testmap.size() == copymap.size());
        REQUIRE(testmap.empty() == false);
        bool passed = true;
        for (int i = 0; i < 1000; i++) {
            if (testmap[i] != i + 1) {
                passed = false;
                break;
            }
        }
        REQUIRE(passed);
    }

    SECTION("copy with iterators with specified larger size")
    {
        std::unordered_map<int, int> copy2{copymap.begin(), copymap.end()};
        TestType testmap{copy2.begin(), copy2.end(), 10000};
        REQUIRE(testmap.bucket_count() >= copymap.bucket_count());
        REQUIRE(testmap.size() == copymap.size());
        REQUIRE(testmap.empty() == false);
        bool passed = true;
        for (int i = 0; i < 1000; i++) {
            if (testmap[i] != copymap[i]) {
                passed = false;
                break;
            }
        }
        REQUIRE(passed);
    }

    SECTION("copy with iterators with specified correct size")
    {
        TestType testmap{copymap.begin(), copymap.end(), 1000};
        REQUIRE(testmap.size() == copymap.size());
        REQUIRE(testmap.empty() == false);
        bool passed = true;
        for (int i = 0; i < 1000; i++) {
            if (testmap[i] != i + 1) {
                passed = false;
                break;
            }
        }
        REQUIRE(passed);
    }

    SECTION("copy with iterators with specified smaller size")
    {
        TestType testmap{copymap.begin(), copymap.end(), 100};
        REQUIRE(testmap.bucket_count() >= copymap.bucket_count());
        REQUIRE(testmap.size() == copymap.size());
        REQUIRE(testmap.empty() == false);
        bool passed = true;
        for (int i = 0; i < 1000; i++) {
            if (testmap[i] != i + 1) {
                passed = false;
                break;
            }
        }
        REQUIRE(passed);
    }

    SECTION("copy with iterators with specified larger size")
    {
        TestType testmap{copymap.begin(), copymap.end(), 2000};
        REQUIRE(testmap.bucket_count() >= copymap.bucket_count());
        REQUIRE(testmap.size() == copymap.size());
        REQUIRE(testmap.empty() == false);
        bool passed = true;
        for (int i = 0; i < 1000; i++) {
            if (testmap[i] != copymap[i]) {
                passed = false;
                break;
            }
        }
        REQUIRE(passed);
    }

    SECTION("COPY construct")
    {
        // delete the copied from map so we know hash_store's internals are correct
        // but we still need copymap to compare to
        TestType* intermediary = new TestType{copymap};
        TestType testmap{*intermediary};
        delete intermediary;
        REQUIRE(testmap.bucket_count() >= copymap.bucket_count());
        REQUIRE(testmap.size() == copymap.size());
        REQUIRE(testmap.empty() == false);
        bool passed = true;
        for (int i = 0; i < 1000; i++) {
            if (testmap[i] != copymap[i]) {
                passed = false;
                break;
            }
        }
        REQUIRE(passed);
    }
    SECTION("MOVE construct")
    {
        TestType another{copymap};
        TestType testmap = std::move(another);
        REQUIRE(testmap.bucket_count() >= copymap.bucket_count());
        REQUIRE(testmap.size() == copymap.size());
        REQUIRE(testmap.empty() == false);
        bool passed = true;
        for (int i = 0; i < 1000; i++) {
            if (testmap[i] != copymap[i]) {
                passed = false;
                break;
            }
        }
        REQUIRE(passed);
        REQUIRE(another.size() == 0);
        REQUIRE(another.empty() == true);
        REQUIRE(another.cbegin() == another.cend());
    }

    SECTION("construct from initializer list")
    {
        TestType testmap({{0, 1}, {1, 2}, {2, 3}});
        REQUIRE(testmap[0] == 1);
        REQUIRE(testmap[1] == 2);
        REQUIRE(testmap[2] == 3);
        REQUIRE(testmap.size() == 3);
    }
    SECTION("construct from initializer list with smaller bucket count")
    {
        TestType testmap({{0, 1}, {1, 2}, {2, 3}}, 1);
        REQUIRE(testmap[0] == 1);
        REQUIRE(testmap[1] == 2);
        REQUIRE(testmap[2] == 3);
        REQUIRE(testmap.size() == 3);
        REQUIRE(testmap.bucket_count() >= 3);
    }
    SECTION("construct from initializer list with larger bucket count")
    {
        TestType testmap({{0, 1}, {1, 2}, {2, 3}}, 10);
        REQUIRE(testmap[0] == 1);
        REQUIRE(testmap[1] == 2);
        REQUIRE(testmap[2] == 3);
        REQUIRE(testmap.size() == 3);
        REQUIRE(testmap.bucket_count() >= 3);
    }

    SECTION("construct from initializer list with correct bucket count")
    {
        TestType testmap({{0, 1}, {1, 2}, {2, 3}}, 3);
        REQUIRE(testmap[0] == 1);
        REQUIRE(testmap[1] == 2);
        REQUIRE(testmap[2] == 3);
        REQUIRE(testmap.size() == 3);
        REQUIRE(testmap.bucket_count() >= 3);
    }
}

TEMPLATE_TEST_CASE("Assignments work", "[assignment]", (std::unordered_map<int, int>), (LP3<int, int>))
{
    TestType copymap;
    TestType empty{};
    for (int i = 0; i < 1000; i++) {
        copymap.insert({i, i + 1});
    }
    SECTION("Copy assignment")
    {
        TestType* intermediary = new TestType{copymap};
        TestType testmap{};
        testmap = *intermediary;
        delete intermediary;
        REQUIRE(testmap == copymap);
    };
    SECTION("move assignment")
    {
        TestType testmap{};
        testmap = TestType{copymap.begin(), copymap.end()};
        REQUIRE(testmap == copymap);
    }
}

TEMPLATE_TEST_CASE("iterators work", "[iterators]", (LP3<int, int>))
{
    TestType empty{};
    TestType testmap({{0, 1}, {1, 2}, {2, 3}}, 3);

    SECTION("constit = it")
    {
        REQUIRE(empty.begin() == empty.cbegin());
        REQUIRE(empty.end() == empty.cend());
        REQUIRE(testmap.begin() == testmap.cbegin());
        REQUIRE(testmap.end() == testmap.cend());
    }
    SECTION("end and begin are the same on empty maps")
    {
        REQUIRE(empty.begin() == empty.end());
        REQUIRE(empty.cbegin() == empty.cend());
    }
    SECTION("Can modify elements with iterator")
    {
        auto it = testmap.begin();
        while (it != testmap.end()) {
            it->second++;
            it++;
        }

        bool incremented_correctly = true;
        for (auto& x : testmap) {
            if (x.first != x.second - 2) {
                incremented_correctly = false;
                break;
            }
        }
        REQUIRE(incremented_correctly == true);
    }

    SECTION("increment and decrement correctly")
    {
        auto it = testmap.begin();
        it++;
        it--;
        ++it;
        --it;
        REQUIRE(it == testmap.begin());
        auto it2 = testmap.cbegin();
        it2++;
        it2--;
        ++it2;
        --it2;
        REQUIRE(it2 == testmap.begin());
    }

    SECTION("derefferencing works")
    {
        std::set<int> contains{0, 1, 2};
        bool working = true;
        auto it = testmap.begin();
        while (it != testmap.end()) {
            if (contains.count((*it++).first) != 1) {
                working = false;
            }
        }
        REQUIRE(working == true);
        auto it2 = testmap.cbegin();
        bool working2 = true;
        while (it2 != testmap.end()) {
            if (contains.count((*it2++).first) != 1) {
                working2 = false;
            }
        }
        REQUIRE(working2 == true);
    }
}

TEMPLATE_TEST_CASE("Modifying stuff", "[modifiers]", (LP3<int, int>), (std::unordered_map<int, int>))
{
    TestType copymap;
    TestType empty{};
    for (int i = 0; i < 1000; i++) {
        copymap.insert({i, i + 1});
    }
    SECTION("Clear")
    {
        TestType testmap{copymap};
        REQUIRE(copymap == testmap);
        testmap.clear();
        REQUIRE(testmap == empty);
        REQUIRE(testmap.empty());
    }
    SECTION("copy insert ")
    {
        TestType testmap{copymap};
        int size = testmap.size();
        Pair_elem testpair{5, 100};
        auto thing = testmap.insert(testpair);
        REQUIRE(thing.second == false);
        REQUIRE(thing.first == testmap.find(testpair.first));
        REQUIRE(testmap.find(testpair.first)->second == 6);
        Pair_elem testpair2{5000, 100};
        auto thing2 = testmap.insert(testpair2);
        REQUIRE(thing2.second == true);
        REQUIRE(thing2.first == testmap.find(testpair2.first));
        REQUIRE(testmap.find(testpair2.first)->second == 100);
        REQUIRE(size + 1 == testmap.size());
    }

    SECTION("copy insert with hint")
    {
        TestType testmap{copymap};
        int size = testmap.size();
        Pair_elem testpair{5, 100};
        auto thing = testmap.insert(testmap.end(), testpair);
        REQUIRE(thing == testmap.find(testpair.first));
        REQUIRE(testmap.find(testpair.first)->second == 6);
        Pair_elem testpair2{5000, 100};
        auto thing2 = testmap.insert(testmap.end(), testpair2);
        REQUIRE(thing2 == testmap.find(testpair2.first));
        REQUIRE(size + 1 == testmap.size());
        REQUIRE(testmap.find(testpair2.first)->second == 100);
    }


    SECTION("move insert with P")
    {
        TestType testmap{copymap};
        int size = testmap.size();
        auto thing = testmap.insert(Pair_float{5.1, 100});
        REQUIRE(thing.second == false);
        REQUIRE(thing.first == testmap.find(5));
        auto thing2 = testmap.insert(Pair_float{5000.1, 100});
        REQUIRE(thing2.second == true);
        REQUIRE(thing2.first == testmap.find(5000));
        REQUIRE(size + 1 == testmap.size());
        REQUIRE(testmap.find(5)->second == 6);
        REQUIRE(testmap.find(5000)->second == 100);
    }
    SECTION("move insert + hint with P")
    {
        TestType testmap{copymap};
        auto hint = testmap.end();
        int size = testmap.size();
        auto thing = testmap.insert(hint, Pair_float{5.1, 100.1});
        REQUIRE(thing == testmap.find(5));
        auto thing2 = testmap.insert(hint, Pair_float{5000.1, 100.1});
        REQUIRE(thing2 == testmap.find(5000));
        REQUIRE(size + 1 == testmap.size());
        REQUIRE(testmap.find(5)->second == 6);
        REQUIRE(testmap.find(5000)->second == 100);
    }

    SECTION("iterator insert")
    {
        TestType testmap{copymap};
        Pair_elem testpair{5, 100};
        Pair_elem testpair2{5000, 100};
        std::vector<Pair_elem> inserts{testpair, testpair2};

        int size = testmap.size();
        testmap.insert(inserts.begin(), inserts.end());
        REQUIRE(testmap.find(5)->second == 6);
        REQUIRE(testmap.find(5000)->second == 100);
        REQUIRE(size + 1 == testmap.size());
    }

    SECTION("Init list insert")
    {
        TestType testmap{copymap};
        Pair_elem testpair{5, 100};
        Pair_elem testpair2{5000, 100};

        int size = testmap.size();
        testmap.insert({testpair, testpair2});
        REQUIRE(testmap.find(5)->second == 6);
        REQUIRE(testmap.find(5000)->second == 100);
        REQUIRE(size + 1 == testmap.size());
    }

#if __cplusplus >= 201703L
    SECTION("Move insert")
    {
        TestType testmap{copymap};
        int size = testmap.size();
        auto thing = testmap.insert({5, 100});
        REQUIRE(thing.second == false);
        REQUIRE(thing.first == testmap.find(5));
        auto thing2 = testmap.insert({5000, 100});
        REQUIRE(thing2.second == true);
        REQUIRE(thing2.first == testmap.find(5000));
        REQUIRE(size + 1 == testmap.size());
        REQUIRE(testmap.find(5)->second == 6);
        REQUIRE(testmap.find(5000)->second == 100);
    }
    SECTION("move insert + hint")
    {
        TestType testmap{copymap};
        auto hint = testmap.end();
        int size = testmap.size();
        auto thing = testmap.insert(hint, {5, 100});
        REQUIRE(thing == testmap.find(5));
        auto thing2 = testmap.insert(hint, {5000, 100});
        REQUIRE(thing2 == testmap.find(5000));
        REQUIRE(size + 1 == testmap.size());
        REQUIRE(testmap.find(5)->second == 6);
        REQUIRE(testmap.find(5000)->second == 100);
    }

    SECTION("insert or assign copy key")
    {
        TestType testmap{copymap};
        int size = testmap.size();
        int k1 = 5;
        int k2 = 5000;
        auto thing = testmap.insert_or_assign(k1, 100.500);
        REQUIRE(thing.second == false);
        REQUIRE(thing.first == testmap.find(5));
        auto thing2 = testmap.insert_or_assign(k2, 100.20);
        REQUIRE(thing2.second == true);
        REQUIRE(thing2.first == testmap.find(5000));
        REQUIRE(testmap.find(5)->second == 100);
        REQUIRE(testmap.find(5000)->second == 100);
        REQUIRE(size + 1 == testmap.size());
    }

    SECTION("insert or assign copy key")
    {
        TestType testmap{copymap};
        int size = testmap.size();
        auto thing = testmap.insert_or_assign(5, 100.500);
        REQUIRE(thing.second == false);
        REQUIRE(thing.first == testmap.find(5));
        auto thing2 = testmap.insert_or_assign(5000, 100.20);
        REQUIRE(thing2.second == true);
        REQUIRE(thing2.first == testmap.find(5000));
        REQUIRE(testmap.find(5)->second == 100);
        REQUIRE(testmap.find(5000)->second == 100);
        REQUIRE(size + 1 == testmap.size());
    }

    SECTION("insert or assign copy key + hint")
    {
        TestType testmap{copymap};
        auto it{testmap.end()};
        int size = testmap.size();
        int k1 = 5;
        int k2 = 5000;
        auto thing = testmap.insert_or_assign(it, k1, 100.500);
        REQUIRE(thing == testmap.find(5));
        auto thing2 = testmap.insert_or_assign(it, k2, 100.20);
        REQUIRE(thing2 == testmap.find(5000));
        REQUIRE(testmap.find(5)->second == 100);
        REQUIRE(testmap.find(5000)->second == 100);
        REQUIRE(size + 1 == testmap.size());
    }

    SECTION("insert or assign copy key + hint")
    {
        TestType testmap{copymap};
        auto it{testmap.end()};
        int size = testmap.size();
        auto thing = testmap.insert_or_assign(it, 5, 100.500);
        REQUIRE(thing == testmap.find(5));
        auto thing2 = testmap.insert_or_assign(it, 5000, 100.20);
        REQUIRE(thing2 == testmap.find(5000));
        REQUIRE(testmap.find(5)->second == 100);
        REQUIRE(testmap.find(5000)->second == 100);
        REQUIRE(size + 1 == testmap.size());
    }
#else
#endif

    SECTION("emplace")
    {
        TestType testmap{copymap};
        int size = testmap.size();
        auto thing = testmap.emplace(5, 100);
        REQUIRE(thing.second == false);
        REQUIRE(thing.first == testmap.find(5));
        auto thing2 = testmap.emplace(5000, 100);
        REQUIRE(thing2.second == true);
        REQUIRE(thing2.first == testmap.find(5000));
        REQUIRE(testmap.find(5)->second == 6);
        REQUIRE(testmap.find(5000)->second == 100);
        REQUIRE(size + 1 == testmap.size());
    }

    SECTION("emplace+ hint")
    {
        TestType testmap{copymap};
        auto it{testmap.end()};
        int size = testmap.size();
        auto thing = testmap.emplace_hint(it, 5, 100);
        REQUIRE(thing == testmap.find(5));
        auto thing2 = testmap.emplace_hint(it, 5000, 100);
        REQUIRE(thing2 == testmap.find(5000));
        REQUIRE(testmap.find(5)->second == 6);
        REQUIRE(testmap.find(5000)->second == 100);
        REQUIRE(size + 1 == testmap.size());
    }

    SECTION("swap")
    {
        TestType copy2{copymap};
        TestType testmap{empty};
        copy2.swap(testmap);
        REQUIRE(copy2 == empty);
        REQUIRE(testmap == copymap);
        std::swap(copy2, testmap);
        REQUIRE(copy2 == copymap);
        REQUIRE(testmap == empty);
    };

    SECTION("erase with value")
    {
        int removed = 0;
        TestType testmap{copymap};
        for (int i = 0; i < 1000; i += 2) {
            removed += testmap.erase(i);
        }
        bool works = true;
        for (int i = 0; i < 1000; i++) {
            if ((i % 2) and (testmap[i] != copymap[i])) {
                works = false;
                break;
            }
            if ((i % 2 == 0) and (testmap.find(i) != testmap.end())) {
                works = false;
                break;
            }
        }
        REQUIRE(works == true);
        REQUIRE(testmap.size() + 500 == copymap.size());
        REQUIRE(removed == 500);
    }
    SECTION("erase with iterator")
    {
        TestType testmap{};
        for (int i = 0; i < 1000; i += 2) {
            auto it = testmap.insert({i, i + 1});
            if (i % 2) {
                testmap.erase(it.first);
            }
        }
        bool works = true;
        for (int i = 0; i < 1000; i++) {
            if ((i % 2) and (testmap.count(i) != 0)) {
                works = false;
                break;
            }
            else if (!(i % 2) and (testmap[i] != i + 1)) {
                works = false;
                break;
            }
        }
        REQUIRE(works == true);
        REQUIRE(testmap.size() + 500 == copymap.size());
    }

    SECTION("erase with iterator")
    {
        TestType testmap{};
        for (int i = 0; i < 1000; i += 2) {
            auto it = testmap.insert({i, i + 1});
            if (i % 2) {
                testmap.erase(it.first);
            }
        }
        bool works = true;
        for (int i = 0; i < 1000; i++) {
            if ((i % 2) and (testmap.count(i) != 0)) {
                works = false;
                break;
            }
            else if (!(i % 2) and (testmap[i] != i + 1)) {
                works = false;
                break;
            }
        }
        REQUIRE(works == true);
        REQUIRE(testmap.size() + 500 == copymap.size());
    }
    SECTION("erase with iterator range")
    {
        TestType testmap{copymap};
        testmap.erase(testmap.cbegin(), testmap.cend());
        REQUIRE(testmap == empty);
    }
    SECTION("operator[] with lval ref key")
    {
        TestType testmap{copymap};
        bool works = true;
        for (int i = 0; i < 1000; i++) {
            if (testmap[i] != i + 1) {
                works = false;
                break;
            }
        }
        REQUIRE(works == true);
        for (int i = 1000; i < 1200; i++) {
            if (testmap[i]) {
                works = false;
                break;
            }
        }
        REQUIRE(works == true);
    }
    SECTION("operator[] with rval ref")
    {
        TestType testmap{copymap};
        bool works = true;
        for (int i = 0; i < 999; i++) {
            if (testmap[i + 1] != i + 2) {
                works = false;
                break;
            }
        }
        REQUIRE(works == true);
        for (int i = 1000; i < 1200; i++) {
            if (testmap[i + 1]) {
                works = false;
                break;
            }
        }
        REQUIRE(works == true);
    }
}
TEMPLATE_TEST_CASE("nonmodifying lookups", "[nonmodifying]", (LP3<int, int>), (std::unordered_map<int, int>))
{
    TestType copymap;
    TestType empty{};
    for (int i = 0; i < 1000; i++) {
        copymap.insert({i, i + 1});
    }

    SECTION("at")
    {
        TestType testmap{copymap};
        bool works = true;
        for (int i = 0; i < 999; i++) {
            if (testmap.at(i) != i + 1) {
                works = false;
                break;
            }
        }
        REQUIRE(works == true);
        REQUIRE_THROWS(testmap.at(5000));
    }
    SECTION("at const")
    {
        TestType testmap{copymap};
        bool works = true;
        for (int i = 0; i < 999; i++) {
            const int a = testmap.at(i);
            if (a != i + 1) {
                works = false;
                break;
            }
        }
        REQUIRE(works == true);
        REQUIRE_THROWS(testmap.at(5000));
    }

    SECTION("count")
    {
        TestType testmap{copymap};
        bool works = true;
        for (int i = 0; i < 999; i++) {
            const int a = testmap.at(i);
            if (a != i + 1) {
                works = false;
                break;
            }
        }
        REQUIRE(works == true);
    }
}
TEMPLATE_TEST_CASE("test that differ from unordered map", "[differ nonmodifying]", (LP3<int, int>))
{
    TestType copymap;
    TestType empty{};
    for (int i = 0; i < 1000; i++) {
        copymap.insert({i, i + 1});
    }
    SECTION("equal range")
    {
        TestType testmap{copymap};
        bool works = true;
        for (int i = 0; i < 999; i++) {
            auto it = testmap.find(i);
            auto pair = std::pair<typename TestType::iterator,typename TestType::iterator>{it, ++it};
            if (testmap.equal_range(i) != pair) {
                works = false;
                break;
            }
        }
        REQUIRE(works == true);
        auto pair = std::pair<typename TestType::iterator,typename TestType::iterator>{testmap.end(), testmap.end()};
        REQUIRE(pair == testmap.equal_range(9000));
    }
    SECTION("hash policy")
    {
        TestType testmap{copymap};
        auto lf = testmap.load_factor();
        auto max_lf = testmap.max_load_factor();
        REQUIRE(lf <= max_lf);
        testmap.max_load_factor(0.01);
        REQUIRE(testmap.load_factor() <= testmap.max_load_factor());
        REQUIRE(testmap.max_load_factor() == 0.01f);
    }
}

TEMPLATE_TEST_CASE("generic shit", "[generic]", (LP3<int, int>), (std::unordered_map<int, int>))
{
    TestType copymap;
    TestType empty{};
    for (int i = 0; i < 1000; i++) {
        copymap.insert({i, i + 1});
    }
    REQUIRE(copymap.max_size() >= INT16_MAX);
    SECTION("observers")
    {
        auto hasher = empty.hash_function();
        auto key_equal = empty.key_eq();
        REQUIRE(hasher(100) > 0);
        REQUIRE(key_equal(100, 100));
    }
}
