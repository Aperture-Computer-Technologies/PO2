//
// Created by MassiveAtoms on 4/28/21.
//
#include <catch2/catch.hpp>
#include <sstream>
#include <unordered_map>

//#include "./../hashmap_implementations/LPmap.h"
#include "./../hashmap_implementations/LPmap3.h"

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

    SECTION("copy with iterators")
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
    SECTION("copy with iterators with specified correct size")
    {
        TestType testmap{copymap.begin(), copymap.end(), 1000};
        //        REQUIRE(testmap.bucket_count() == copymap.bucket_count());
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
        TestType testmap{copymap};
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