cmake_minimum_required(VERSION 3.7...3.19)

if (${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif ()

project(PO2 VERSION 1.0
        DESCRIPTION "project ontwerp 2"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
# add "-fsanitize=address -fno-omit-frame-pointer"
# to detect memory errors (leaks, double deletes, etc)

# "-fsanitize=memory -fPIE -pie -fsanitize-memory-track-origins -fno-omit-frame-pointer "
#to check if you're reading uninitialized memory

# "-fsanitize=undefined -fno-omit-frame-pointer"
# checks if anything you wrote relies on undefined behaviour
# which means that your program prolly won't behave exactly as you want on a different compiler

set(CMAKE_CXX_FLAGS "-o3 -march=native")
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

# check the cmake stuff at the json lib you're using for cmake help


# Sagar's Linear Probing
add_executable(sagar-test
        ${PROJECT_SOURCE_DIR}/test/hashmap_test.cpp
        )

add_executable(LP-profiling
        ${PROJECT_SOURCE_DIR}/hashmap_implementations/profiler.cpp
        )

 target_link_libraries(LP-profiling
         profiler

         )
# tests with catch2
find_package(Catch2 REQUIRED)
add_executable(better-test
       test/better_tests.cpp test/better_test_speed.cpp)
target_link_libraries(better-test PRIVATE Catch2::Catch2)


# hashmap benchmakrs
add_executable(benchmarks
        ./benchmarks/includes/generator.h
        ./benchmarks/generator.cpp
        ./benchmarks/includes/tests.h
        ./benchmarks/includes/aggregate_tests.h
        ./benchmarks/bench.cpp
        )

add_executable(snippet-run
        ./hashmap_implementations/snippet-tester.cpp
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

