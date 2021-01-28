#include "./hash.h"

#include <iostream>

#include "./hash-test.h"

int main()
{
    hash_test_suite(example, "naive_modulo");
    hash_test_suite(dumb_bitmask, "naive_bitmask");
    hash_test_suite(sfold, "sfold");
}