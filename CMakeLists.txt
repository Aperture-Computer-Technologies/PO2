cmake_minimum_required(VERSION 3.15)
project(PO2 VERSION 1.0
                  DESCRIPTION "project ontwerp 2"
                  LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O3 -flto=thin -march=native")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Process Abseil's CMake build system
# add_subdirectory(
#     ./src/includes/3thparty/abseil-cpp # example

#                 )


add_executable(hash-test 
                ./test/hash.h
                ./test/hash-test.cpp
                ./test/random.h
                ./test/hash-test.h
)

# target_link_libraries(studproject 
# #                         absl::hash 

#                         pthread 
#                         dl
#                         stdc++fs
# )

