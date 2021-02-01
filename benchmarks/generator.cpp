
#include "./includes/generator.h"  // imports a generator to be used for the functions

// generates string to be used as a key
std::string gen_string()
{  // 90^size posibilities
    std::string randomstring;
    for (int i = 0; i < 5; ++i) {
        randomstring += singlechar(generator);
    }
    return randomstring;
}
// gen strings that dont exist in the hashmap
std::string gen_unsuccesfull_string()
{  // 90^size posibilities
    std::string randomstring;
    for (int i = 0; i < 4; ++i) {
        randomstring += singlechar(generator);  //
    }
    return randomstring;
}

// gen integers to be used as keys
int gen_int() { return insert_int(generator); }
// gen ints that don't exist in the hashmap
int gen_unsuccesfull_int() { return noninsert_int(generator); }
