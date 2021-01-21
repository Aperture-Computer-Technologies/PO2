#include "chaining.h"
#include <unordered_map>
#include <chrono>

int main(){
    chaining test;
    test.insert({1,500});
    test.insert({2,500});
    cout << test[1] << "," << test[20] << "\n";
    test.erase(1);
    cout << test[1] << "," << test[2];



}