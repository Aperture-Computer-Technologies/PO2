//
// Created by MassiveAtoms on 4/7/21.
//
#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "LPmap3.h"
using namespace std;
template <typename K, typename V>
ostream& operator<<(ostream& os, pair<K, V> obj)
{
    os << "{" << obj.first << ", " << obj.second << "}\n";
    return os;
}

using Pair_elem = std::pair<const int, int>;
using TestType = LP3<int, int>;

int main()
{
    //
// Created by MassiveAtoms on 4/28/21.
//
#include <iostream>
//#include <sstream>
//#include <unordered_map>

//#include "./../hashmap_implementations/LPmap.h"
#include "./../hashmap_implementations/LPmap3.h"

    {
        // fill a hashmap with 1000 values, for constructors that copy/move or otherwise construct from another hashmap
        TestType copymp;
        for (int i = 0; i < 1000; i++) {
            copymp.insert({i, i + 1});
        }
        const TestType copymap{copymp};

        {
            TestType testmap{copymap};
            TestType copy2{copymap};
            TestType copy3{copymap};
            copy2.insert({8000, 7});
            copy3[5] = 999;
            cout << ((copymap == testmap) == true);
            cout << ((copy2 == testmap) == false);
            cout << ((copy3 == testmap) == false);
            cout << ((copymap != testmap) != (copymap == testmap));
            cout << ((copy2 != testmap) != (copy2 == testmap));
            cout << ((copy3 != testmap) != (copy3 == testmap));
        }
    }

    using Pair_elem = std::pair<const int, int>;
    using std::cout;
    {
        // fill a hashmap with 1000 values, for constructors that copy/move or otherwise construct from another hashmap
        TestType copymap;
        for (int i = 0; i < 1000; i++) {
            copymap.insert({i, i + 1});
        }

        {
            TestType testmap{};
            cout << (testmap.bucket_count() > 0);
            cout << (testmap.size() == 0);
            cout << (testmap.empty() == true);
            bool iter_empty = (testmap.begin() == testmap.end() && testmap.cbegin() == testmap.cend());
            cout << (iter_empty);
        }
        {
            TestType testmap{250};
            cout << (testmap.bucket_count() > 250);
            cout << (testmap.size() == 0);
            cout << (testmap.empty() == true);
            TestType map2{};
            map2.reserve(250);
            cout << (sizeof(testmap) == sizeof(map2));
            bool iter_empty = (testmap.begin() == testmap.end() && testmap.cbegin() == testmap.cend());
            cout << (iter_empty);
        }
        {
            TestType testmap{250, std::allocator<Pair_elem>()};
            cout << (testmap.get_allocator() == std::allocator<Pair_elem>());
            cout << (testmap.bucket_count() > 250);
            cout << (testmap.size() == 0);
            cout << (testmap.empty() == true);
            TestType map2{};
            map2.reserve(250);
            cout << (sizeof(testmap) == sizeof(map2));
            bool iter_empty = (testmap.begin() == testmap.end() && testmap.cbegin() == testmap.cend());
            cout << (iter_empty);
        }

        {
            auto hasher = std::hash<int>{};
            TestType testmap{250, hasher, std::allocator<Pair_elem>()};
            cout << (testmap.get_allocator() == std::allocator<Pair_elem>());
            cout << (testmap.bucket_count() > 250);
            cout << (testmap.size() == 0);
            cout << (testmap.empty() == true);
            TestType map2{};
            map2.reserve(250);
            cout << (sizeof(testmap) == sizeof(map2));
            bool iter_empty = (testmap.begin() == testmap.end() && testmap.cbegin() == testmap.cend());
            cout << (iter_empty);
        }

        {
            TestType testmap{250, std::allocator<Pair_elem>()};
            cout << (testmap.get_allocator() == std::allocator<Pair_elem>());
            cout << (testmap.bucket_count() > 250);
            cout << (testmap.size() == 0);
            cout << (testmap.empty() == true);
            TestType map2{};
            map2.reserve(250);
            cout << (sizeof(testmap) == sizeof(map2));
            bool iter_empty = (testmap.begin() == testmap.end() && testmap.cbegin() == testmap.cend());
            cout << (iter_empty);
        }

        {
            TestType testmap{copymap.begin(), copymap.end()};
            cout << (testmap.bucket_count() == copymap.bucket_count());
            cout << (testmap.size() == copymap.size());
            cout << (testmap.empty() == false);
            bool passed = true;
            for (int i = 0; i < 1000; i++) {
                if (testmap[i] != i + 1) {
                    passed = false;
                    break;
                }
            }
            cout << (passed);
        }
        {
            std::unordered_map<int, int> copy2{copymap.begin(), copymap.end()};
            TestType testmap{copy2.begin(), copy2.end()};
            cout << (testmap.bucket_count() == copymap.bucket_count());
            cout << (testmap.size() == copymap.size());
            cout << (testmap.empty() == false);
            bool passed = true;
            for (int i = 0; i < 1000; i++) {
                if (testmap[i] != i + 1) {
                    passed = false;
                    break;
                }
            }
            cout << (passed);
        }

        {
            std::unordered_map<int, int> copy2{copymap.begin(), copymap.end()};
            TestType testmap{copy2.begin(), copy2.end(), 1000};
            cout << (testmap.size() == copy2.size());
            cout << (testmap.empty() == false);
            bool passed = true;
            for (int i = 0; i < 1000; i++) {
                if (testmap[i] != i + 1) {
                    passed = false;
                    break;
                }
            }
            cout << (passed);
        }

        {
            std::unordered_map<int, int> copy2{copymap.begin(), copymap.end()};
            TestType testmap{copy2.begin(), copy2.end(), 10};
            cout << (testmap.bucket_count() >= copymap.bucket_count());
            cout << (testmap.size() == copymap.size());
            cout << (testmap.empty() == false);
            bool passed = true;
            for (int i = 0; i < 1000; i++) {
                if (testmap[i] != i + 1) {
                    passed = false;
                    break;
                }
            }
            cout << (passed);
        }

        {
            std::unordered_map<int, int> copy2{copymap.begin(), copymap.end()};
            TestType testmap{copy2.begin(), copy2.end(), 10000};
            cout << (testmap.bucket_count() >= copymap.bucket_count());
            cout << (testmap.size() == copymap.size());
            cout << (testmap.empty() == false);
            bool passed = true;
            for (int i = 0; i < 1000; i++) {
                if (testmap[i] != copymap[i]) {
                    passed = false;
                    break;
                }
            }
            cout << (passed);
        }

        {
            TestType testmap{copymap.begin(), copymap.end(), 1000};
            cout << (testmap.size() == copymap.size());
            cout << (testmap.empty() == false);
            bool passed = true;
            for (int i = 0; i < 1000; i++) {
                if (testmap[i] != i + 1) {
                    passed = false;
                    break;
                }
            }
            cout << (passed);
        }

        {
            TestType testmap{copymap.begin(), copymap.end(), 100};
            cout << (testmap.bucket_count() >= copymap.bucket_count());
            cout << (testmap.size() == copymap.size());
            cout << (testmap.empty() == false);
            bool passed = true;
            for (int i = 0; i < 1000; i++) {
                if (testmap[i] != i + 1) {
                    passed = false;
                    break;
                }
            }
            cout << (passed);
        }

        {
            TestType testmap{copymap.begin(), copymap.end(), 2000};
            cout << (testmap.bucket_count() >= copymap.bucket_count());
            cout << (testmap.size() == copymap.size());
            cout << (testmap.empty() == false);
            bool passed = true;
            for (int i = 0; i < 1000; i++) {
                if (testmap[i] != copymap[i]) {
                    passed = false;
                    break;
                }
            }
            cout << (passed);
        }

        {
            // delete the copied from map so we know hash_store's internals are correct
            // but we still need copymap to compare to
            TestType* intermediary = new TestType{copymap};
            TestType testmap{*intermediary};
            delete intermediary;
            cout << (testmap.bucket_count() >= copymap.bucket_count());
            cout << (testmap.size() == copymap.size());
            cout << (testmap.empty() == false);
            bool passed = true;
            for (int i = 0; i < 1000; i++) {
                if (testmap[i] != copymap[i]) {
                    passed = false;
                    break;
                }
            }
            cout << (passed);
        }

        {
            TestType another{copymap};
            TestType testmap = std::move(another);
            cout << (testmap.bucket_count() >= copymap.bucket_count());
            cout << (testmap.size() == copymap.size());
            cout << (testmap.empty() == false);
            bool passed = true;
            for (int i = 0; i < 1000; i++) {
                if (testmap[i] != copymap[i]) {
                    passed = false;
                    break;
                }
            }
            cout << (passed);
            cout << (another.size() == 0);
            cout << (another.empty() == true);
            cout << (another.cbegin() == another.cend());
        }

        {
            TestType testmap({{0, 1}, {1, 2}, {2, 3}});
            cout << (testmap[0] == 1);
            cout << (testmap[1] == 2);
            cout << (testmap[2] == 3);
            cout << (testmap.size() == 3);
        }

        {
            TestType testmap({{0, 1}, {1, 2}, {2, 3}}, 1);
            cout << (testmap[0] == 1);
            cout << (testmap[1] == 2);
            cout << (testmap[2] == 3);
            cout << (testmap.size() == 3);
            cout << (testmap.bucket_count() >= 3);
        }

        {
            TestType testmap({{0, 1}, {1, 2}, {2, 3}}, 10);
            cout << (testmap[0] == 1);
            cout << (testmap[1] == 2);
            cout << (testmap[2] == 3);
            cout << (testmap.size() == 3);
            cout << (testmap.bucket_count() >= 3);
        }

        {
            TestType testmap({{0, 1}, {1, 2}, {2, 3}}, 3);
            cout << (testmap[0] == 1);
            cout << (testmap[1] == 2);
            cout << (testmap[2] == 3);
            cout << (testmap.size() == 3);
            cout << (testmap.bucket_count() >= 3);
        }
    }

    {
        TestType copymap;
        TestType empty{};
        for (int i = 0; i < 1000; i++) {
            copymap.insert({i, i + 1});
        }

        {
            TestType* intermediary = new TestType{copymap};
            TestType testmap{};
            testmap = *intermediary;
            delete intermediary;
            cout << (testmap == copymap);
        };

        {
            TestType testmap{};
            testmap = TestType{copymap.begin(), copymap.end()};
            cout << (testmap == copymap);
        }
    }

    {
        TestType empty{};
        TestType testmap({{0, 1}, {1, 2}, {2, 3}}, 3);

        {
            cout << (empty.begin() == empty.cbegin());
            cout << (empty.end() == empty.cend());
            cout << (testmap.begin() == testmap.cbegin());
            cout << (testmap.end() == testmap.cend());
        }

        {
            cout << (empty.begin() == empty.end());
            cout << (empty.cbegin() == empty.cend());
        }

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
            cout << (incremented_correctly == true);
        }

        {
            auto it = testmap.begin();
            it++;
            it--;
            ++it;
            --it;
            cout << (it == testmap.begin());
            auto it2 = testmap.cbegin();
            it2++;
            it2--;
            ++it2;
            --it2;
            cout << (it2 == testmap.begin());
        }

        {
            std::set<int> contains{0, 1, 2};
            bool working = true;
            auto it = testmap.begin();
            while (it != testmap.end()) {
                if (contains.count((*it++).first) != 1) {
                    working = false;
                }
            }
            cout << (working == true);
            auto it2 = testmap.cbegin();
            bool working2 = true;
            while (it2 != testmap.end()) {
                if (contains.count((*it2++).first) != 1) {
                    working2 = false;
                }
            }
            cout << (working2 == true);
        }
    }

    {
        TestType copymap;
        TestType empty{};
        for (int i = 0; i < 1000; i++) {
            copymap.insert({i, i + 1});
        }

        {
            TestType testmap{copymap};
            cout << (copymap == testmap);
            testmap.clear();
            cout << (testmap == empty);
            cout << (testmap.empty());
        }

        {
            TestType testmap{copymap};
            int size = testmap.size();
            Pair_elem testpair{5, 100};
            auto thing = testmap.insert(testpair);
            cout << (thing.second == false);
            cout << (thing.first == testmap.find(testpair.first));
            cout << (testmap.find(testpair.first)->second == 6);
            Pair_elem testpair2{5000, 100};
            auto thing2 = testmap.insert(testpair2);
            cout << (thing2.second == true);
            cout << (thing2.first == testmap.find(testpair2.first));
            cout << (testmap.find(testpair2.first)->second == 100);
            cout << (size + 1 == testmap.size());
        }

        {
            TestType testmap{copymap};
            int size = testmap.size();
            Pair_elem testpair{5, 100};
            auto thing = testmap.insert(testmap.end(), testpair);
            cout << (thing == testmap.find(testpair.first));
            cout << (testmap.find(testpair.first)->second == 6);
            Pair_elem testpair2{5000, 100};
            auto thing2 = testmap.insert(testmap.end(), testpair2);
            cout << (thing2 == testmap.find(testpair2.first));
            cout << (size + 1 == testmap.size());
            cout << (testmap.find(testpair2.first)->second == 100);
        }

        using Pair_float = std::pair<const float, float>;

        {
            TestType testmap{copymap};
            int size = testmap.size();
            auto thing = testmap.insert(Pair_float{5.1, 100});
            cout << (thing.second == false);
            cout << (thing.first == testmap.find(5));
            auto thing2 = testmap.insert(Pair_float{5000.1, 100});
            cout << (thing2.second == true);
            cout << (thing2.first == testmap.find(5000));
            cout << (size + 1 == testmap.size());
            cout << (testmap.find(5)->second == 6);
            cout << (testmap.find(5000)->second == 100);
        }

        {
            TestType testmap{copymap};
            auto hint = testmap.end();
            int size = testmap.size();
            auto thing = testmap.insert(hint, Pair_float{5.1, 100.1});
            cout << (thing == testmap.find(5));
            auto thing2 = testmap.insert(hint, Pair_float{5000.1, 100.1});
            cout << (thing2 == testmap.find(5000));
            cout << (size + 1 == testmap.size());
            cout << (testmap.find(5)->second == 6);
            cout << (testmap.find(5000)->second == 100);
        }

        {
            TestType testmap{copymap};
            Pair_elem testpair{5, 100};
            Pair_elem testpair2{5000, 100};
            std::vector<Pair_elem> inserts{testpair, testpair2};

            int size = testmap.size();
            testmap.insert(inserts.begin(), inserts.end());
            cout << (testmap.find(5)->second == 6);
            cout << (testmap.find(5000)->second == 100);
            cout << (size + 1 == testmap.size());
        }

        {
            TestType testmap{copymap};
            Pair_elem testpair{5, 100};
            Pair_elem testpair2{5000, 100};

            int size = testmap.size();
            testmap.insert({testpair, testpair2});
            cout << (testmap.find(5)->second == 6);
            cout << (testmap.find(5000)->second == 100);
            cout << (size + 1 == testmap.size());
        }

#if __cplusplus >= 201703L

        {
            TestType testmap{copymap};
            int size = testmap.size();
            auto thing = testmap.insert({5, 100});
            cout << (thing.second == false);
            cout << (thing.first == testmap.find(5));
            auto thing2 = testmap.insert({5000, 100});
            cout << (thing2.second == true);
            cout << (thing2.first == testmap.find(5000));
            cout << (size + 1 == testmap.size());
            cout << (testmap.find(5)->second == 6);
            cout << (testmap.find(5000)->second == 100);
        }

        {
            TestType testmap{copymap};
            auto hint = testmap.end();
            int size = testmap.size();
            auto thing = testmap.insert(hint, {5, 100});
            cout << (thing == testmap.find(5));
            auto thing2 = testmap.insert(hint, {5000, 100});
            cout << (thing2 == testmap.find(5000));
            cout << (size + 1 == testmap.size());
            cout << (testmap.find(5)->second == 6);
            cout << (testmap.find(5000)->second == 100);
        }

        {
            TestType testmap{copymap};
            int size = testmap.size();
            int k1 = 5;
            int k2 = 5000;
            auto thing = testmap.insert_or_assign(k1, 100.500);
            cout << (thing.second == false);
            cout << (thing.first == testmap.find(5));
            auto thing2 = testmap.insert_or_assign(k2, 100.20);
            cout << (thing2.second == true);
            cout << (thing2.first == testmap.find(5000));
            cout << (testmap.find(5)->second == 100);
            cout << (testmap.find(5000)->second == 100);
            cout << (size + 1 == testmap.size());
        }

        {
            TestType testmap{copymap};
            int size = testmap.size();
            auto thing = testmap.insert_or_assign(5, 100.500);
            cout << (thing.second == false);
            cout << (thing.first == testmap.find(5));
            auto thing2 = testmap.insert_or_assign(5000, 100.20);
            cout << (thing2.second == true);
            cout << (thing2.first == testmap.find(5000));
            cout << (testmap.find(5)->second == 100);
            cout << (testmap.find(5000)->second == 100);
            cout << (size + 1 == testmap.size());
        }

        {
            TestType testmap{copymap};
            auto it{testmap.end()};
            int size = testmap.size();
            int k1 = 5;
            int k2 = 5000;
            auto thing = testmap.insert_or_assign(it, k1, 100.500);
            cout << (thing == testmap.find(5));
            auto thing2 = testmap.insert_or_assign(it, k2, 100.20);
            cout << (thing2 == testmap.find(5000));
            cout << (testmap.find(5)->second == 100);
            cout << (testmap.find(5000)->second == 100);
            cout << (size + 1 == testmap.size());
        }

        {
            TestType testmap{copymap};
            auto it{testmap.end()};
            int size = testmap.size();
            auto thing = testmap.insert_or_assign(it, 5, 100.500);
            cout << (thing == testmap.find(5));
            auto thing2 = testmap.insert_or_assign(it, 5000, 100.20);
            cout << (thing2 == testmap.find(5000));
            cout << (testmap.find(5)->second == 100);
            cout << (testmap.find(5000)->second == 100);
            cout << (size + 1 == testmap.size());
        }
#else
#endif

        {
            TestType testmap{copymap};
            int size = testmap.size();
            auto thing = testmap.emplace(5, 100);
            cout << (thing.second == false);
            cout << (thing.first == testmap.find(5));
            auto thing2 = testmap.emplace(5000, 100);
            cout << (thing2.second == true);
            cout << (thing2.first == testmap.find(5000));
            cout << (testmap.find(5)->second == 6);
            cout << (testmap.find(5000)->second == 100);
            cout << (size + 1 == testmap.size());
        }

        {
            TestType testmap{copymap};
            auto it{testmap.end()};
            int size = testmap.size();
            auto thing = testmap.emplace_hint(it, 5, 100);
            cout << (thing == testmap.find(5));
            auto thing2 = testmap.emplace_hint(it, 5000, 100);
            cout << (thing2 == testmap.find(5000));
            cout << (testmap.find(5)->second == 6);
            cout << (testmap.find(5000)->second == 100);
            cout << (size + 1 == testmap.size());
        }

        {
            TestType copy2{copymap};
            TestType testmap{empty};
            copy2.swap(testmap);
            cout << (copy2 == empty);
            cout << (testmap == copymap);
            std::swap(copy2, testmap);
            cout << (copy2 == copymap);
            cout << (testmap == empty);
        };

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
            cout << (works == true);
            cout << (testmap.size() + 500 == copymap.size());
            cout << (removed == 500);
        }

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
            cout << (works == true);
            cout << (testmap.size() + 500 == copymap.size());
        }

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
            cout << (works == true);
            cout << (testmap.size() + 500 == copymap.size());
        }

        {
            TestType testmap{copymap};
            testmap.erase(testmap.cbegin(), testmap.cend());
            cout << (testmap == empty);
        }

        {
            TestType testmap{copymap};
            bool works = true;
            for (int i = 0; i < 1000; i++) {
                if (testmap[i] != i + 1) {
                    works = false;
                    break;
                }
            }
            cout << (works == true);
            for (int i = 1000; i < 1200; i++) {
                if (testmap[i]) {
                    works = false;
                    break;
                }
            }
            cout << (works == true);
        }

        {
            TestType testmap{copymap};
            bool works = true;
            for (int i = 0; i < 999; i++) {
                if (testmap[i + 1] != i + 2) {
                    works = false;
                    break;
                }
            }
            cout << (works == true);
            for (int i = 1000; i < 1200; i++) {
                if (testmap[i + 1]) {
                    works = false;
                    break;
                }
            }
            cout << (works == true);
        }
    }
    {
        TestType copymap;
        TestType empty{};
        for (int i = 0; i < 1000; i++) {
            copymap.insert({i, i + 1});
        }

        {
            TestType testmap{copymap};
            bool works = true;
            for (int i = 0; i < 999; i++) {
                if (testmap.at(i) != i + 1) {
                    works = false;
                    break;
                }
            }
            cout << (works == true);
        }

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
            cout << (works == true);
        }

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
            cout << (works == true);
        }
    }
    {
        TestType copymap;
        TestType empty{};
        for (int i = 0; i < 1000; i++) {
            copymap.insert({i, i + 1});
        }

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
            cout << (works == true);
            auto pair = std::pair<typename TestType::iterator,typename TestType::iterator>{testmap.end(), testmap.end()};
            cout << (pair == testmap.equal_range(9000));
        }

        {
            TestType testmap{copymap};
            auto lf = testmap.load_factor();
            auto max_lf = testmap.max_load_factor();
            cout << (lf <= max_lf);
            testmap.max_load_factor(0.01);
            cout << (testmap.load_factor() <= testmap.max_load_factor());
            cout << (testmap.max_load_factor() == 0.01f);
        }
    }

    {
        TestType copymap;
        TestType empty{};
        for (int i = 0; i < 1000; i++) {
            copymap.insert({i, i + 1});
        }
        cout << (copymap.max_size() >= INT16_MAX);

        {
            auto hasher = empty.hash_function();
            auto key_equal = empty.key_eq();
            cout << (hasher(100) > 0);
            cout << (key_equal(100, 100));
        }
    }
}