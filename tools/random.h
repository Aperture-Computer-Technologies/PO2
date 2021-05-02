#ifndef RANDOMH
#define RANDOMH

#include <random>
#include <string>
// this generates bytes, and we use a seed so it stays deterministic
static std::mt19937 gener(INT32_MAX - 2020);
// these are distributions. they take the bytes that generator outputs
// and do stuff with it so that every output of these has an equally likely chance of getting
// outputted
static std::uniform_int_distribution<int> random_int_distr(1, INT32_MAX);
static std::uniform_int_distribution<int> singlech(33, 123);

std::string gen_str() {  // 90^inserted posibilities
  std::string randomstring;
  for (int i = 0; i < 5; ++i) {
      randomstring += (char)singlech(gener);
  }
  return randomstring;
}

// gen integers between 0 and int32max
int gen_integer() { return random_int_distr(gener); }

#endif