cmake_minimum_required(VERSION 3.7...3.19)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

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

# for testing hash functions
add_executable(hash-test 
./test/hash.h
        ./test/hash-test.cpp
        ./tools/random.h
        ./test/hash-test.h
        ./external/json.hpp
        )

# Sagar's Linear Probing
#add_executable(LP-test2
#        ${PROJECT_SOURCE_DIR}/test/hashmap_test.cpp
#        hashmap_implementations/Pmap.h)

# Sagar's Linear Probing
add_executable(LP-test
        ${PROJECT_SOURCE_DIR}/test/hashmap_test.cpp
        hashmap_implementations/LPmap.h)


# compile quick snippets
add_executable(snippet_test
        ./hashmap_implementations/snippets.cpp
        hashmap_implementations/Pmap.h)

# cuckoo hashing implementation, rajeev 
add_executable(cuckoo-impl
        ./hashmap_implement/cuckoo_hashing.h
        ./hashmap_implement/cuckoo.cpp
        )

# hashmap benchmakrs
add_executable(benchmarks
        ./benchmarks/includes/generator.h
        ./benchmarks/generator.cpp
        ./benchmarks/includes/tests.h
        ./benchmarks/includes/aggregate_tests.h
        ./benchmarks/main.cpp
        )

# target_link_libraries(benchmarks
#         pthread
#         dl
#         stdc++fs
#         )

# target_link_libraries(studproject 
# #                         absl::hash 

#                         pthread 
#                         dl
#                         stdc++fs
# )

