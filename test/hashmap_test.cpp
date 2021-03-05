#include "./hashmap_test.h"

#include <unordered_map>

#include "./../hashmap_implementations/LPmap2.h"
#include "./../hashmap_implementations/LPmap.h"
#include "./../hashmap_implementations/nodemap.h"
#include "./../hashmap_implementations/nodemap1b.h"
#include "./../hashmap_implementations/nodemap2.h"

int main(){
//    hashmap_test_suite(std::unordered_map<int, int> {});
    cout << "--------Sagar's LP map\n";
    hashmap_test_suite(LP<int,int>{});
    cout << "--------Sagar's LP2 map\n";
    hashmap_test_suite(LP2<int,int>{});
    cout << "--------Sagar's nodemap\n";
    hashmap_test_suite(Nodemap<int,int>{});
        cout << "--------Sagar's node1b map\n";
    hashmap_test_suite(Nodemap1b<int,int>{});
    cout << "--------Sagar's node2 map\n";
    hashmap_test_suite(Nodemap2<int,int>{});

}