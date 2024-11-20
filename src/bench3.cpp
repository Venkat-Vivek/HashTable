#include <benchmark/benchmark.h>
#include <iostream>
#include <chrono>
#include <string>
#include <thread>
#include <unordered_map>
#include <functional>
#include <vector>
#include "hashTable.h" 
// #include "partitionLockHashTable.h"
// #include "blockLockHashTable.h" 
// #include "blockLockHashTableAtomic.h" 

using namespace std;

// std::mutex mapMutex;

unordered_map<int, int> mp;

int numElements=100000;

HashTable<int, int> hashTable;
// Hashtable<int, int> hashTable1;
// BlockLockHashTable<int, int> hashTable2;

void benchmarkInsertion(){
    for (int i = 1; i <= numElements; ++i) {
        hashTable.insert(i, i + 1);
    }
}

void benchmarkInsertion1(){
    for (int i = 1; i <= numElements; ++i) {
        // std::lock_guard<std::mutex> mapLock(mapMutex);
        mp.insert({i, i + 1});
    }
}

void benchmarkLookUp(){
    for (int i = 1; i <= numElements; ++i) {
        hashTable.get(i);
    }
}

void benchmarkLookUp1(){
    for (int i = 1; i <= numElements; ++i) {
        // std::lock_guard<std::mutex> mapLock(mapMutex);
        mp.at(i);
    }
}

void benchmarkUpdate(){
    for (int i = 1; i <= numElements; ++i) {
        hashTable.update(i, i + 2);
    }
}

void benchmarkUpdate1(){
    for (int i = 1; i <= numElements; ++i) {
        // std::lock_guard<std::mutex> mapLock(mapMutex);
        mp[i]=i + 2;
    }
}


void benchmarkDelete(){
    for (int i = 1; i <= numElements; ++i) {
        hashTable.remove(i);
    }
}

void benchmarkDelete1(){
    for (int i = 1; i <= numElements; ++i) {
        // std::lock_guard<std::mutex> mapLock(mapMutex);
        mp.erase(i);
    }
}
static void UNORDERED(benchmark::State& state)
{
    for (auto _ : state)
    {
        benchmarkInsertion1();
    }
}

static void CUSTOM(benchmark::State& state)
{
    for (auto _ : state)
    {
        benchmarkInsertion();
    }
}

static void UNORDERED_GET(benchmark::State& state)
{
    for (auto _ : state)
    {
        benchmarkLookUp1();
    }
}

static void CUSTOM_GET(benchmark::State& state)
{
    for (auto _ : state)
    {
        benchmarkLookUp();
    }
}

static void UNORDERED_UPDATE(benchmark::State& state)
{
    for (auto _ : state)
    {
        benchmarkUpdate1();
    }
}

static void CUSTOM_UPDATE(benchmark::State& state)
{
    for (auto _ : state)
    {
        benchmarkUpdate();
    }
}

static void UNORDERED_DELETE(benchmark::State& state)
{
    for (auto _ : state)
    {
        benchmarkDelete1();
    }
}

static void CUSTOM_DELETE(benchmark::State& state)
{
    for (auto _ : state)
    {
        benchmarkDelete();
    }
}

BENCHMARK(UNORDERED);
BENCHMARK(CUSTOM);

BENCHMARK(UNORDERED_GET);
BENCHMARK(CUSTOM_GET);

BENCHMARK(UNORDERED_UPDATE);
BENCHMARK(CUSTOM_UPDATE);

BENCHMARK(UNORDERED_DELETE);
BENCHMARK(CUSTOM_DELETE);

BENCHMARK_MAIN();
