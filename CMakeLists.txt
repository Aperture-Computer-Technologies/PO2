cmake_minimum_required(VERSION 3.15)
project(PO2 VERSION 1.0
                  DESCRIPTION "project ontwerp 2"
                  LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O3 -flto=thin -march=native")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CLANG_TIDY_CXX -checks=-*,
    bugprone-*,
    cert-*,
    clang-analyzer-*,
    concurrency-*,
    cppcoreguidelines-*,
    hicpp-*,
    llvm-*,
    misc-*,
    modernize-*,
    mpi-*,
    openmp-*,
    performance-*,
    portability-*,
    readability-*,
)
set(CLANG_TIDY_SUPPRESS_CXX
    misc-*,
)
# Process Abseil's CMake build system
# add_subdirectory(
#     ./src/includes/3thparty/abseil-cpp # example
#                 )

# check the cmake stuff at the json lib you're using for cmake help


add_executable(hash-test 
                ./test/hash.h
                ./test/hash-test.cpp
                ./test/random.h
                ./test/hash-test.h
                ./external/json.hpp
)

add_executable(cuckoo-impl
 ./hashmap_implement/cuckoo_hashing.h
 ./hashmap_implement/cuckoo.cpp
 
)

# target_link_libraries(studproject 
# #                         absl::hash 

#                         pthread 
#                         dl
#                         stdc++fs
# )

