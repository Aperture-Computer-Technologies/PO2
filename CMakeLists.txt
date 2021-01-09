cmake_minimum_required(VERSION 3.15)
project(another_studproject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O3 -flto=thin -march=native")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Process Abseil's CMake build system
# add_subdirectory(
#     ./src/includes/3thparty/abseil-cpp # example

#                 )


add_executable(studproject 
                ./test/hashtesting.h
                ./test/test_hash.cpp

#                 ./src/includes/generator.h
#                 ./src/generator.cpp
#                 ./src/includes/tests.h
#                 ./src/includes/aggregate_tests.h
#                 ./src/includes/3thparty/emilib/loguru.cpp
#                 ./src/main.cpp
)

target_link_libraries(studproject 
#                         absl::hash 
#                         absl::node_hash_map 
#                         absl::flat_hash_map 
                        pthread 
                        dl
                        stdc++fs
)

