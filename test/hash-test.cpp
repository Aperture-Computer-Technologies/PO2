#include "./hash.h"
#include <iostream>
#include "./hash-test.h"

int main() {
    std::cout << chi_squared_test_rand(example, 2 << 10) << "\n";
    std::cout << chi_squared_test_wiki(example, 2 << 10) << "\n";
    std::cout << bit_histogram(example, 2 << 10) << "\n";

}