
#ifndef GENERATOR_H
#define GENERATOR_H

#include <random>
#include <string>
// this generates bytes, and we use a seed so it stays deterministic
static std::mt19937 generator(INT32_MAX - 2020);
// these are distributions. they take the bytes that generator outputs
// and do stuff with it so that every output of these has an equally likely chance of getting outputted
static std::uniform_int_distribution<int> insert_int(1, INT32_MAX * 0.875);
static std::uniform_int_distribution<int> noninsert_int(INT32_MAX * 0.875, INT32_MAX);
static std::uniform_int_distribution<int> singlechar(33, 123);

// see generator.cpp for more detail, but the names are pretty self explanatory
int gen_int();

int gen_unsuccesfull_int();

std::string gen_string();

std::string gen_unsuccesfull_string();

#endif /* GENERATOR_H */