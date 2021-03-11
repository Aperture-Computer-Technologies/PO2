//
// Created by MassiveAtoms on 3/11/21.
//

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "./../hashmap_implementations/LPmap.h"
#include "./../tools/random.h"
using namespace std::chrono;

template <typename T>
constexpr auto type_name()
{
    std::string_view name, prefix, suffix;
#ifdef __clang__
    name = __PRETTY_FUNCTION__;
    prefix = "auto type_name() [T = ";
    suffix = "]";
#elif defined(__GNUC__)
    name = __PRETTY_FUNCTION__;
    prefix = "constexpr auto type_name() [with T = ";
    suffix = "]";
#elif defined(_MSC_VER)
    name = __FUNCSIG__;
    prefix = "auto __cdecl type_name<";
    suffix = ">(void)";
#endif
    name.remove_prefix(prefix.size());
    name.remove_suffix(suffix.size());
    return name;
}

template <class T>
std::basic_string_view<char> name(T var)
{
    return type_name<decltype(var)>();
}

std::vector<int> sizes = {50000,
                          100000,
                          150000,
                          200000,
                          250000,
                          300000,
                          350000,
                          400000,
                          500000,
                          600000,
                          700000,
                          800000,
                          900000,
                          1000000,
                          2000000,
                          3000000,
                          4000000,
                          5000000};

template <typename T>
std::vector<int> rehash_bencher(T map, int maxsize)
{
    T testmap{};
    std::vector<int> times;
    for (auto& size : sizes) {
        {
            if (size > maxsize){
                break;
            }
            std::vector<int> insertions(size - testmap.size());
            std::generate(insertions.begin(), insertions.end(), gen_integer);
            testmap.reserve(size);
            for (const auto& x : insertions) {
                testmap.insert({x, x});
            }
            insertions.clear();
            insertions.shrink_to_fit();
        }

        time_point<steady_clock> start = steady_clock::now();
        testmap.rehash();
        time_point<steady_clock> end = steady_clock::now();
        auto rehash_time = duration_cast<milliseconds>(end - start);
        times.emplace_back(rehash_time.count());
    }
    std::cout << testmap[gen_integer()];
    testmap.clear();
    return times;
}

template <typename T>
void bench_output(int n, T map)
{
    for (int i = 0; i < n; i++) {
        std::string oline = "rehash, " + std::string(name(map));
        auto res = rehash_bencher(map, 2000000);
        for (auto x : res) {
            oline += (", " + std::to_string(x));
        }
        std::ofstream output{"results.csv", std::ios_base::app};
        output << oline << "\n";
        std::cout << oline << "\n";
    }
}

int main()
{
    bench_output(2, LP<int, int>{});
}