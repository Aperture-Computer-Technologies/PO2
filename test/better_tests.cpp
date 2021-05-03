//
// Created by MassiveAtoms on 4/28/21.
//
#include <catch2/catch.hpp>

#include <iostream>
#include <set>
#include <unordered_map>
#include <algorithm>

#include "./../hashmap_implementations/LPmap3.h"

using Pair_elem = std::pair<const int, int>;
using Pair_float = std::pair<const float, float>;
using namespace std;
class odd{
  public:
    odd(){};
    bool operator()(Pair_elem x){return x.first % 2;};
};


TEMPLATE_TEST_CASE("std::algorithms work", "[equality]", (std::unordered_map<int, int>), (LP3<int, int>)){
    TestType copymap;
    for (int i = 0; i < 1000; i++) {
        copymap.insert({i, i + 1});
    }
    TestType only_even{};
    for (int i = 0; i < 100; i++){
        only_even.insert({i*2, i});
    }
    TestType empty;

    auto is_odd = odd{};
    SECTION("nonmodifiing"){
        int only_even_res = count_if(only_even.begin(), only_even.end(), is_odd);
        int empty_res = count_if(empty.begin(), empty.end(), is_odd);
        int copymap_res = count_if(copymap.begin(), copymap.end(), is_odd);
        REQUIRE(only_even_res == 0);
        REQUIRE(empty_res == 0);
        REQUIRE(copymap_res == 500);
    }
    SECTION("modifying"){
        vector<Pair_elem> only_odd {};
        copy_if(copymap.begin(), copymap.end(), back_inserter(only_odd), is_odd);
        vector<Pair_elem> leeg{};
        copy_if(empty.begin(), empty.end(), back_inserter(leeg), is_odd);
        copy_if(only_even.begin(), only_even.end(), back_inserter(leeg), is_odd);
        int counter = count_if(only_odd.begin(), only_odd.end(), is_odd);
        REQUIRE(leeg.size() ==0);
        REQUIRE(counter == 500);
        REQUIRE(counter == only_odd.size());
    }

}