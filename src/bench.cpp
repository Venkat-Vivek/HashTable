#include <benchmark/benchmark.h>
#include <iostream>
#include <chrono>
#include <string>
#include <thread>
#include <unordered_map>
#include <functional>
#include <vector>
#include "hashTable.h" 
#include "partitionLockHashTable.h"
// #include "blockLockHashTable.h" 
// #include "blockLockHashTableAtomic.h" 

using namespace std;

std::mutex mapMutex;

unordered_map<int, int> mp;

int numElements = 10000;

HashTable<int, int> hashTable;
CustomHashTable<int, int> hashTable1;

void benchmarkInsertion(){
    for (int i = 1; i <= numElements; ++i) {
        hashTable.insert(i, i + 1);
    }
}

void benchmarkInsertion1(){
    for (int i = 1; i <= numElements; ++i) {
        std::lock_guard<std::mutex> mapLock(mapMutex);
        mp.insert({i, i + 1});
    }
}

void benchmarkInsertion2(){
    for (int i = 1; i <= numElements; ++i) {
        hashTable1.insert(i, i + 1);
    }
}


// void benchmarkLookup(){
//     for (int i = 1; i <= numElements; ++i) {
//         hashTable.get(i);
//     }
// }


// void benchmarkLookup1(){
//     for (int i = 1; i <= numElements; ++i) {
//         std::lock_guard<std::mutex> mapLock(mapMutex);
//         mp.at(i);
//     }
// }


// void benchmarkLookup2(){
//     for (int i = 1; i <= numElements; ++i) {
//         hashTable1.get(i);
//     }
// }

void test(int count, const std::function<void()>& benchmarkMethod){    
    thread t[count];
    for(int i = 0; i < count; i++){
        t[i] = thread(benchmarkMethod);
    }
    for (int i = 0; i < count; i++){
        t[i].join();
    }
}

static void UNORDERED(benchmark::State& state)
{
    for (auto _ : state)
    {
        test(8, benchmarkInsertion1);
    }
}

static void PARTITION(benchmark::State& state)
{
    for (auto _ : state)
    {
        test(8, benchmarkInsertion);
    }
}

static void CUSTOM(benchmark::State& state)
{
    for (auto _ : state)
    {
        test(8, benchmarkInsertion2);
    }
}

BENCHMARK(UNORDERED);
BENCHMARK(CUSTOM);
BENCHMARK(PARTITION);

BENCHMARK_MAIN();