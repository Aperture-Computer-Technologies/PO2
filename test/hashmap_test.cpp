#include "./hashmap_test.h"

#include <unordered_map>

#include "./../hashmap_implementations/LPmap.h"
#include "./../hashmap_implementations/nodemap.h"
int main(){
//    hashmap_test_suite(std::unordered_map<int, int> {});
    cout << "--------Sagar's LP map\n";
//    hashmap_test_suite(LPmap{});
    Nodemap<int,int> test{};
    cout << test.size();


    hashmap_test_suite(Nodemap<int,int>{});

}