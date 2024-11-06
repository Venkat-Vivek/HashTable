cmake_minimum_required(VERSION 3.10)
project(HashTable)

# Add Google Benchmark as a subdirectory
add_subdirectory(benchmark)
# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the benchmark library
find_package(benchmark REQUIRED)

# Set the sources with the correct path
set(SOURCES ../src/benchmark.cpp ../src/murmurHash.cpp)

# Define the executable
add_executable(HashTable ${SOURCES})

# Link the executable with benchmark and pthread libraries
target_link_libraries(HashTable benchmark::benchmark pthread)
