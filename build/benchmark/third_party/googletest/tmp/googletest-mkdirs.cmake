# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/venkat-pt7718/sample/HashTable/benchmark/googletest"
  "/home/venkat-pt7718/sample/HashTable/build/benchmark/third_party/googletest/build"
  "/home/venkat-pt7718/sample/HashTable/build/benchmark/third_party/googletest"
  "/home/venkat-pt7718/sample/HashTable/build/benchmark/third_party/googletest/tmp"
  "/home/venkat-pt7718/sample/HashTable/build/benchmark/third_party/googletest/src/googletest-stamp"
  "/home/venkat-pt7718/sample/HashTable/build/benchmark/third_party/googletest/download"
  "/home/venkat-pt7718/sample/HashTable/build/benchmark/third_party/googletest/src/googletest-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/venkat-pt7718/sample/HashTable/build/benchmark/third_party/googletest/src/googletest-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/venkat-pt7718/sample/HashTable/build/benchmark/third_party/googletest/src/googletest-stamp${cfgdir}") # cfgdir has leading slash
endif()
