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


add_executable(hash-test 
./test/hash.h
./test/hash-test.cpp
./tools/random.h
./test/hash-test.h
./external/json.hpp
)


add_executable(chaining
${PROJECT_SOURCE_DIR}/chaining_hashmap/chaining.h
${PROJECT_SOURCE_DIR}/chaining_hashmap/chaining.cpp
)

# target_link_libraries(studproject 
# #                         absl::hash 

#                         pthread 
#                         dl
#                         stdc++fs
# )

