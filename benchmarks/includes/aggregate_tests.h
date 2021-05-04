#ifndef ATESTS_H
#define ATESTS_H

#include <fstream>

#include "./tests.h"

// sizes that will be tested
vector<int> sizes
    = {50000,    100000,   150000,   200000,   250000,   300000,   350000,   400000,   500000,  600000,  700000,
       800000,   900000,   1000000,  2000000,  3000000,  4000000,  5000000,  6000000,  7000000, 8000000, 9000000,
       10000000, 15000000, 20000000, 25000000, 30000000, 35000000, 40000000, 45000000, 50000000};

/*
to print typenames
for more info, see
https://stackoverflow.com/a/20170989
and
https://stackoverflow.com/a/56766138
*/

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

/*
This is the function that outputs the results to a file.
it calls int_test (n_run) times for all sizes < maxsize, and then append the
results to the outputfile.
We use a template function to massively reduce the ammount of code needed.
Instead of writing 17 different functions for all different hashmaps, we can
do this. The compiler will then see that a call is made where the map is of type
std::unordered_map, for example, and then generate the function where T is
replaced with std::unordered_map. Well, that's the simple explanation,
more info at https://en.cppreference.com/w/cpp/language/templates
*/

template <class T>
void int_test_aggregate(T map, int runs, int maxsize = 20000000)
{
    std::ofstream output{"results.csv", std::ios_base::app};
    for (int i = 0; i < runs; ++i) {
        string insert = "\nint_insert, \"";
        string succ_lookup = "\nint_succ_lookup, \"";
        string nosucc_lookup = "\nint_nosucc_lookup, \"";
        string delet = "\nint_delete, \"";
        string iter = "\nint_iter, \"";

        insert += string{name(map)} + "\"";
        succ_lookup += string{name(map)} + "\"";
        nosucc_lookup += string{name(map)} + "\"";
        delet += string{name(map)} + "\"";
        iter += string{name(map)} + "\"";
        for (auto size : sizes) {
            if (size > maxsize) {
                break;
            }
            vector<long int> results = int_test<T>(size);

            insert += ", " + std::to_string(results[0]);
            succ_lookup += ", " + std::to_string(results[1]);
            nosucc_lookup += ", " + std::to_string(results[2]);
            delet += ", " + std::to_string(results[3]);
            iter += ", " + std::to_string(results[4]);
        }
        output << insert << succ_lookup << nosucc_lookup << delet << iter;
        cout << insert << succ_lookup << nosucc_lookup << delet << iter;
    }
}

/*
This is pretty much the same function, but it calls string_test instead of
int_test. More info on why we needed to split this, can be seen in tests.h
*/
template <class T>
void string_test_aggregate(T map, int runs, int maxsize = 20000000)
{
    std::ofstream output{"results.csv", std::ios_base::app};
    for (int i = 0; i < runs; ++i) {
        string insert = "\nstring_insert, \"";
        string succ_lookup = "\nstring_succ_lookup, \"";
        string nosucc_lookup = "\nstring_nosucc_lookup, \"";
        string delet = "\nstring_delete, \"";
        string iter = "\nstring_iter, \"";

        insert += string{name(map)} + "\"";
        succ_lookup += string{name(map)} + "\"";
        nosucc_lookup += string{name(map)} + "\"";
        delet += string{name(map)} + "\"";
        iter += string{name(map)} + "\"";
        for (auto size : sizes) {
            if (size > maxsize) {
                break;
            }
            vector<long int> results = string_test<T>(size);

            insert += ", " + std::to_string(results[0]);
            succ_lookup += ", " + std::to_string(results[1]);
            nosucc_lookup += ", " + std::to_string(results[2]);
            delet += ", " + std::to_string(results[3]);
            iter += ", " + std::to_string(results[4]);
        }
        output << insert << succ_lookup << nosucc_lookup << delet << iter;
        cout << insert << succ_lookup << nosucc_lookup << delet << iter;
    }
}

#endif