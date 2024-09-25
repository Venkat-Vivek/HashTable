cmake_minimum_required(VERSION 3.10)

project(HashTable)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(HashTable
    src/main.cpp         
    src/murmurHash.cpp    
)

target_include_directories(HashTable PUBLIC "${PROJECT_SOURCE_DIR}/src")
