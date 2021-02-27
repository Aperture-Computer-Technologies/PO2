#include <iostream>

#include "./includes/3thparty/CLI11.hpp"
#include "./../hashmap_implementations/LPmap.h"
#include "./includes/aggregate_tests.h"
// include your implementations

string choicetext
    = "Select implementation to test: '-i 1 2 3 4' or '-i "
      "1,2,3'. Default is all  \n"
      "1. std::unordered_hashmap(1) \n"
      "2. boost::unordered\n";

// default arguments
vector<int> hashmaps = {1, 2,3};
int runs = 1;
int maxsize = 5000000;

/*
int_test_aggregate and string_test_aggregate are called for different hashmaps
based on the choices
see implementation of these in includes/aggregate_tests
*/
int main(int argc, char **argv)
{
    // This is just stuff to add options like selecting which hashmaps need to get
    // benchmarked
    CLI::App app{"Hashmap benchmarks"};
    app.add_option("-i,--implementation", hashmaps, choicetext)->delimiter(',');
    app.add_option("-r,--runs", runs, "total runs for each map, default is 1");
    app.add_option("-m, --maxsize", maxsize, "The max inserted of the hashmaps to test for. Default is 50 million.");
    CLI11_PARSE(app, argc, argv);
    time_point<steady_clock> start_test = steady_clock::now();
    // calls int_test_aggregate and it's string version for different hashmaps
    // based on the choices selected

    for (auto i : hashmaps) {
        switch (i) {
            case 1: {
                int_test_aggregate(std::unordered_map<int, int>{}, runs, maxsize);
//                string_test_aggregate(std::unordered_map<string, string>{}, runs, maxsize); // TODO: renable
                break;
            }
            case 2: {
                int_test_aggregate(boost::unordered_map<int, int>{}, runs, maxsize);
//                string_test_aggregate(boost::unordered_map<string, string>{}, runs, maxsize); // TODO: reenable
                break;
            }
            case 3: {
                int_test_aggregate(LPmap{}, runs, maxsize);
            }
        }

        time_point<steady_clock> end_test = steady_clock::now();
        std::cout << "\n\n"
                  << runs << " runs for all tests for " << hashmaps.size()
                  << "maps: " << duration_cast<seconds>(end_test - start_test).count() << " seconds\n\n";
    }
}