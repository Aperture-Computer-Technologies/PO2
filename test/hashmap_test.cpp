#include "./hashmap_test.h"

#include <unordered_map>

#include "./../hashmap_implementations/chaining-simd.h"
int main(){
    hashmap_test_suite(std::unordered_map<int, int> {});
    cout << "--------Sagar's Chaining map\n";
    hashmap_test_suite(chaining{});

}