#include <benchmark/benchmark.h>
#include <iostream>
#include <chrono>
#include <string>
#include <thread>
#include <unordered_map>
#include <functional>
#include <shared_mutex>
#include <vector>
#include "hashTable.h" 
#include "partitionLockHashTable.h"
#include "blockLockHashTable.h" 
// #include "blockLockHashTableAtomic.h" 

using namespace std;

std::shared_mutex mapMutex;

unordered_map<int, int> mp;

int numElements = 100000;

HashTable<int, int> hashTable;
Hashtable<int, int> hashTable1;
BlockLockHashTable<int, int> hashTable2;

vector<int> v, vv;

void numElementsFun(){
    for(int i = 1; i <= numElements; i++){
        v.push_back(rand() % numElements);
        vv.push_back(rand() % numElements);
    }
}

void benchmarkInsertion(){
    for (int i = 1; i <= numElements; ++i) {
        int el = v[i - 1];
        hashTable.insert(el, el + 1);
    }
}

void benchmarkInsertion1(){
    for (int i = 1; i <= numElements; ++i) {
        std::unique_lock<std::shared_mutex> mapLock(mapMutex);
        int el = v[i - 1];
        mp.insert({el, el + 1});
    }
}

void benchmarkInsertion2(){
    for (int i = 1; i <= numElements; ++i) {
        int el = v[i - 1];
        hashTable1.insert(el, el + 1);
    }
}

void benchmarkInsertion3(){
    for (int i = 1; i <= numElements; ++i) {
        int el = v[i - 1];
        hashTable2.insert(el, el + 1);
    }
}

void benchmark0(){
    for (int i = 1; i <= numElements; ++i) {
        int el = v[i - 1];
        hashTable.insert(el, el + 1);
        hashTable.get(vv[i - 1]);
    }
}

void benchmark1(){
    for (int i = 1; i <= numElements; ++i) {
        {
            std::unique_lock<std::shared_mutex> mapLock(mapMutex);
            int el = v[i - 1];
            mp.insert({el, el + 1});
        }
        {
            std::shared_lock<std::shared_mutex> mapLock(mapMutex);
            mp[vv[i - 1]];
        }
    }
}

void benchmark2(){
    for (int i = 1; i <= numElements; ++i) {
        int el = v[i - 1];
        hashTable1.insert(el, el + 1);
        hashTable1.get(vv[i - 1]);
    }
}

void benchmark3(){
    for (int i = 1; i <= numElements; ++i) {
        int el = v[i - 1];
        hashTable2.insert(el, el + 1);
        hashTable2.get(vv[i - 1]);
    }
}


void benchmarkLookup(){
    for (int i = 1; i <= numElements; ++i) {
        hashTable.get(vv[i - 1]);
    }
}


void benchmarkLookup1(){
    for (int i = 1; i <= numElements; ++i) {
        std::shared_lock<std::shared_mutex> mapLock(mapMutex);
        mp[vv[i - 1]];
    }
}


void benchmarkLookup2(){
    for (int i = 1; i <= numElements; ++i) {
        hashTable1.get(vv[i - 1]);
    }
}

void benchmarkLookup3(){
    for (int i = 1; i <= numElements; ++i) {
        hashTable2.get(vv[i - 1]);
    }
}

// void benchmarkInsertion2(){
//     int numElements=100000;
//     for (int i = 1; i <= numElements; ++i) {
//         hashTable2.insert(i, i + 1);
//     }
// }


// void benchmarkUpdation(){
//     int numElements=10000;
//     for (int i = 1; i <= numElements; ++i) {
//         hashTable.update(i, i + numElements);
//     }
// }

// void benchmarkRemoval(){
//     int numElements=10000;
//     for (int i = 1; i <= numElements; ++i) {
//         hashTable.remove(i);
//     }
// }

// concurrent_hash_map<int, int> mp;

// void benchmarkInsertion2() {
//     int numElements = 10000;
//     for (int i = 1; i <= numElements; ++i) {
//         concurrent_hash_map<int, int>::accessor accessor;
//         mp.insert(accessor, {i, i + 1});  // Insert elements safely
//     }
// }

// void benchmarkLookup2() {
//     int numElements = 10000;
//     for (int i = 1; i <= numElements; ++i) {
//         concurrent_hash_map<int, int>::const_accessor const_accessor;
//         if (mp.find(const_accessor, i)) {
//             int value = const_accessor->second;  // Access the value
//             // Optionally use 'value' here if needed
//         } else {
//             cout << "Key " << i << " not found." << endl;
//         }
//     }
// }

// void benchmarkUpdation2() {
//     int numElements = 10000;
//     for (int i = 1; i <= numElements; ++i) {
//         concurrent_hash_map<int, int>::accessor accessor;
//         if (mp.find(accessor, i)) {
//             accessor->second = i + numElements;  // Update the value
//         } else {
//             cout << "Key " << i << " not found for update." << endl;
//         }
//     }
// }

// void benchmarkRemoval2() {
//     int numElements = 10000;
//     for (int i = 1; i <= numElements; ++i) {
//         mp.erase(i);  // Erase by key
//     }
// }
void test(int count, const std::function<void()>& benchmarkMethod){    
    thread t[count];
    for(int i = 0; i < count; i++){
        t[i] = thread(benchmarkMethod);
    }
    for (int i = 0; i < count; i++){
        t[i].join();
        // this_thread::sleep_for(1ms);
    }
}

// static void BENCHMARK_HASHTABLE(benchmark::State& state)
// {
//     for (auto _ : state)
//     {
//         test(16, benchmarkHashTable);
//     }
// }

int noOfThreads = 16;

static void NUMELEMENTS(benchmark::State& state)
{
    for (auto _ : state)
    {
        numElementsFun();
    }
}


static void UNORDERED(benchmark::State& state)
{
    for (auto _ : state)
    {
        test(noOfThreads, benchmarkInsertion1);
    }
}

static void PARTITION(benchmark::State& state)
{
    for (auto _ : state)
    {
        test(noOfThreads, benchmarkInsertion);
    }
}

static void CUSTOM(benchmark::State& state)
{
    for (auto _ : state)
    {
        test(noOfThreads, benchmarkInsertion2);
    }
}

static void BLOCK(benchmark::State& state)
{
    for (auto _ : state)
    {
        test(noOfThreads, benchmarkInsertion3);
    }
}

static void UNORDERED2(benchmark::State& state)
{
    for (auto _ : state)
    {
        test(noOfThreads, benchmark1);
    }
}

static void PARTITION2(benchmark::State& state)
{
    for (auto _ : state)
    {
        test(noOfThreads, benchmark0);
    }
}

static void CUSTOM2(benchmark::State& state)
{
    for (auto _ : state)
    {
        test(noOfThreads, benchmark2);
    }
}

static void BLOCK2(benchmark::State& state)
{
    for (auto _ : state)
    {
        test(noOfThreads, benchmark3);
    }
}

static void UNORDERED_GET(benchmark::State& state)
{
    for (auto _ : state)
    {
        test(noOfThreads, benchmarkLookup1);
    }
}

static void PARTITION_GET(benchmark::State& state)
{
    for (auto _ : state)
    {
        test(noOfThreads, benchmarkLookup);
    }
}

static void CUSTOM_GET(benchmark::State& state)
{
    for (auto _ : state)
    {
        test(noOfThreads, benchmarkLookup2);
    }
}

static void BLOCK_GET(benchmark::State& state)
{
    for (auto _ : state)
    {
        test(noOfThreads, benchmarkLookup3);
    }
}

// static void INSERTION2(benchmark::State& state)
// {
//     for (auto _ : state)
//     {
//         test(2, benchmarkInsertion2);
//     }
// }

// static void LOOKUP(benchmark::State& state)
// {
//     for (auto _ : state)
//     {
//         test(1, benchmarkLookup);
//     }
// }

// static void UPDATION(benchmark::State& state)
// {
//     for (auto _ : state)
//     {
//         test(1,benchmarkUpdation);
//     }
// }

// static void DELETION(benchmark::State& state)
// {
//     for (auto _ : state)
//     {
//         test(1,benchmarkRemoval);
//     }
// }

// static void INSERTION2(benchmark::State& state)
// {
//     for (auto _ : state)
//     {
//         test(4, benchmarkInsertion2);
//     }
// }

// static void LOOKUP2(benchmark::State& state)
// {
//     for (auto _ : state)
//     {
//         test(4, benchmarkLookup2);
//     }
// }

// static void UPDATION2(benchmark::State& state)
// {
//     for (auto _ : state)
//     {
//         test(4,benchmarkUpdation2);
//     }
// }

// static void DELETION2(benchmark::State& state)
// {
//     for (auto _ : state)
//     {
//         test(4,benchmarkRemoval2);
//     }
// }

BENCHMARK(NUMELEMENTS);
BENCHMARK(UNORDERED);
BENCHMARK(CUSTOM);
BENCHMARK(PARTITION);
BENCHMARK(BLOCK);

BENCHMARK(UNORDERED_GET);
BENCHMARK(CUSTOM_GET);
BENCHMARK(PARTITION_GET);
BENCHMARK(BLOCK_GET);

BENCHMARK(UNORDERED2);
BENCHMARK(CUSTOM2);
BENCHMARK(PARTITION2);
BENCHMARK(BLOCK2);
// BENCHMARK(LOOKUP);
// BENCHMARK(UPDATION);
// BENCHMARK(DELETION);
// BENCHMARK(BENCHMARK_HASHTABLE);

//partition hash table
// BENCHMARK(LOOKUP);
// BENCHMARK(UPDATION);
// BENCHMARK(DELETION);
// BENCHMARK(BENCHMARK_HASHTABLE);

// //unordered map
// BENCHMARK(INSERTION2);
// BENCHMARK(LOOKUP2);
// BENCHMARK(UPDATION2);
// BENCHMARK(DELETION2);

BENCHMARK_MAIN();
// void benchmarkHashTable() {
//     try{
//         HashTable<int, int> hashTable;

//         int numElements=1000;
//         for (int i = 1; i <= numElements; ++i) {
//             hashTable.insert(i,i+1);
//         }
        
//         for (int i = 1; i <= numElements; ++i) {
//             int ans = hashTable.get(i);
//         }
        
//         for (int i = 1; i <= numElements; ++i) {
//             bool flag = hashTable.update(i,i+numElements);
//         }

//         // for (int i = 1; i <= numElements; ++i) {
//         //     bool flag = hashTable.remove(i);
//         // }
       
//     }
//      catch (const exception& e) {
//         cout << "Exception " << e.what() << endl;
//     }
// }

// void benchmarkHashTable2() {
//     try {
//         // Define the concurrent hash map
//         concurrent_hash_map<int, int> mp;
//         int numElements = 1000;

//         // Insert elements using an accessor
//         for (int i = 1; i <= numElements; ++i) {
//             concurrent_hash_map<int, int>::accessor accessor;
//             mp.insert(accessor, {i, i + 1});
//         }

//         // Retrieve elements using a const accessor
//         for (int i = 1; i <= numElements; ++i) {
//             concurrent_hash_map<int, int>::const_accessor const_accessor;
//             if (mp.find(const_accessor, i)) {
//                 int ans = const_accessor->second;  // Access the value
//                 // Optionally use 'ans' here if needed
//             }
//         }

//         // Update elements using an accessor
//         for (int i = 1; i <= numElements; ++i) {
//             concurrent_hash_map<int, int>::accessor accessor;
//             if (mp.find(accessor, i)) {
//                 accessor->second = i + numElements;  // Update the value
//             }
//         }

//         // Erase elements
//         for (int i = 1; i <= numElements; ++i) {
//             mp.erase(i);  // Erase by key
//         }
//     }
//     catch (const exception& e) {
//         cout << "Exception: " << e.what() << endl;
//     }
// }

// void benchmarkHashTable2() {
//     try{
//         int numElements=1000;
//         for (int i = 1; i <= numElements; ++i) {
//             mp.insert({i,i+1});
//         }
        
//         for (int i = 1; i <= numElements; ++i) {
//             int ans = mp[i];
//         }
        
//         for (int i = 1; i <= numElements; ++i) {
//             mp[i]=i+numElements;
//         }

//         for (int i = 1; i <= numElements; ++i) {
//             mp.erase(i);
//         }
       
//     }
//     catch (const exception& e) {
//         cout << "Exception " << e.what() << endl;
//     }
// }

// void test6( int count){ 
//     thread t[count];
//     for(int i = 0; i < count; i++){
//         t[i] = thread(benchmarkHashTable);
//     }
//     for (int i = 0; i <count; i++){
//         t[i].join();
//     }
// }

// void test7( int count){    

//     thread t[count];
//     for(int i = 0; i < count; i++){
//         t[i] = thread(benchmarkHashTable2);
//     }
//     for (int i = 0; i <count; i++){
//         t[i].join();
//     }
// }



// static void BENCHMARK_HASHTABLE(benchmark::State& state)
// {   
//     for (auto _ : state)
//     {
//         test6(1000);
//     }
// }


// static void BENCHMARK_HASHTABLE2(benchmark::State& state)
// {   
//     for (auto _ : state)
//     {
//         test7(1000);
//     }
// }


// BENCHMARK(BENCHMARK_HASHTABLE); // custom
// BENCHMARK(BENCHMARK_HASHTABLE2); //unordered_map

// BENCHMARK_MAIN();



// void test1(){
//     // cout << "Test 1" << endl;
//     // cout << "---------- " << endl;
//     HashTable<int, int> hashTable;
//     hashTable.remove(0);
//         // cout << "Successfully deleted" << endl;
//     // else 
//         // cout << "Could not found " << endl;
//     // cout << hashTable.get(0) << endl;
//     hashTable.update(0, 1);
//         // cout << "Successfully updated" << endl;
    
//         // cout << "Could not found " << endl;
//     hashTable.printHashTable();


//     HashTable<string, int> hashTable1;
//     hashTable1.remove("");
//         // cout << "Successfully deleted" << endl;

//         // cout << "Could not found " << endl;
//     // cout << hashTable1.get("") << endl;
//     hashTable1.update("", 1);
//         // cout << "Successfully updated" << endl;
//     // else 
//         // cout << "Could not found " << endl;
//     // hashTable1.printHashTable();

//     // cout << endl;
// }



// // void test2(){
// //     cout << "Test 2" << endl;
// //     cout << "---------- " << endl;
// //     HashTable<int, int> hashTable;
// //     if(hashTable.remove(1))
// //         cout << "Successfully deleted" << endl;
// //     else 
// //         cout << "Could not found " << endl;
// //     cout << hashTable.get(1) << endl;
// //     if(hashTable.update(1, 2))
// //         cout << "Successfully updated" << endl;
// //     else 
// //         cout << "Could not found " << endl;
// //     hashTable.printHashTable();


// //     HashTable<string, int> hashTable1;
// //     if(hashTable1.remove("hai"))
// //         cout << "Successfully deleted" << endl;
// //     else 
// //         cout << "Could not found " << endl;
// //     cout << hashTable1.get("") << endl;
// //     if(hashTable1.update("hai", 1))
// //         cout << "Successfully updated" << endl;
// //     else 
// //         cout << "Could not found " << endl;
// //     hashTable1.printHashTable();

// //     cout << endl;
// // }

// // void test3(){
// //     cout << "Test 3" << endl;
// //     cout << "---------- " << endl;
// //     HashTable<int, int> hashTable;
// //     hashTable.insert(0, 0);

// //     cout << hashTable.get(0) << endl;
// //     if(hashTable.update(0, 2))
// //         cout << "Successfully updated" << endl;
// //     else 
// //         cout << "Could not found " << endl;
// //     if(hashTable.remove(0))
// //         cout << "Successfully deleted" << endl;
// //     else 
// //         cout << "Could not found " << endl;
// //     cout << hashTable.get(0) << endl;
// //     hashTable.printHashTable();

// //     HashTable<string, int> hashTable1;
// //     hashTable1.insert("", 7);
// //     cout << hashTable1.get("") << endl;
// //     if(hashTable1.update("", 1))
// //         cout << "Successfully updated" << endl;
// //     else 
// //         cout << "Could not found " << endl;
// //     if(hashTable1.remove(""))
// //         cout << "Successfully deleted" << endl;
// //     else 
// //         cout << "Could not found " << endl;
// //     hashTable1.printHashTable();

// //     cout << endl;
// // }

// // void test4(){
// //     cout << "Test 4" << endl;
// //     cout << "---------- " << endl;
// //     HashTable<int, int> hashTable;
// //     hashTable.insert(17, 0);

// //     cout << hashTable.get(17) << endl;
// //     if(hashTable.update(17, 19))
// //         cout << "Successfully updated" << endl;
// //     else 
// //         cout << "Could not found " << endl;
// //     cout << hashTable.get(17) << endl;
// //     if(hashTable.remove(17))
// //         cout << "Successfully deleted" << endl;
// //     else 
// //         cout << "Could not found " << endl;
// //     cout << hashTable.get(17) << endl;
// //     hashTable.printHashTable();

// //     HashTable<string, int> hashTable1;
// //     hashTable1.insert("hai", 25);
// //     cout << hashTable1.get("hai") << endl;
// //     if(hashTable1.update("hai", 8))
// //         cout << "Successfully updated" << endl;
// //     else 
// //         cout << "Could not found " << endl;
// //     if(hashTable1.remove("hai"))
// //         cout << "Successfully deleted" << endl;
// //     else 
// //         cout << "Could not found " << endl;
// //     hashTable1.printHashTable();
// //     cout << endl;
// // }

// // void test5(){
// //     cout << "Test 5" << endl;
// //     cout << "---------- " << endl;
// //     HashTable<int, int> hashTable;
// //     hashTable.insert(17, 8);
// //     hashTable.insert(17, 5);
// //     cout << hashTable.get(17) << endl;
// //     hashTable.remove(17);
// //     hashTable.insert(17, 25);
// //     cout << hashTable.get(17) << endl;
// //     hashTable.printHashTable();

// //     HashTable<string, int> hashTable1;
// //     hashTable1.insert("sai", 19);
// //     cout << hashTable1.get("sai") << endl;
// //     hashTable1.insert("sai", 15);
// //     hashTable1.remove("sai");
// //     cout << hashTable1.get("sai") << endl;
// //     hashTable1.insert("sai", 10);
// //     cout << hashTable1.get("sai") << endl;
// //     hashTable1.printHashTable();

// //     cout << endl;
// // }



// // Register the function as a benchmark
// // Run the benchmark
// BENCHMARK_MAIN();
   

//    // This code gets timed
//  //empty hash table & accessing updating deleting default values
//     // test1();
//     // //empty hash table & accessing updating deleting normal values 
//     // test2();
//     // //inserting default values into hash table & accessing updating deleting them
//     // test3();
//     // //inserting normal values into hash table & accessing updating deleting them
//     // test4();
//     // //inserting duplicate keys into hash table & accessing updating deleting them
//     // test5();
//     // //multithreading
//     // test6();
//     // int numElements = 1000; 
//     // benchmarkHashTable(numElements);

// #include <benchmark/benchmark.h>
// #include <iostream>

// static void BM_BenchHashTable(benchmark::State& state) {
//   // Perform setup here
//   for (auto _ : state) {
//     cout << "Test 1" << endl;
//     // cout << "---------- " << endl;
//     // HashTable<int, int> hashTable;
//     // if(hashTable.remove(0))
//     //     cout << "Successfully deleted" << endl;
//     // else 
//     //     cout << "Could not found " << endl;
//     // cout << hashTable.get(0) << endl;
//     // if(hashTable.update(0, 1))
//     //     cout << "Successfully updated" << endl;
//     // else 
//     //     cout << "Could not found " << endl;
//     // hashTable.printHashTable();


//     // HashTable<string, int> hashTable1;
//     // if(hashTable1.remove(""))
//     //     cout << "Successfully deleted" << endl;
//     // else 
//     //     cout << "Could not found " << endl;
//     // cout << hashTable1.get("") << endl;
//     // if(hashTable1.update("", 1))
//     //     cout << "Successfully updated" << endl;
//     // else 
//     //     cout << "Could not found " << endl;
//     // hashTable1.printHashTable();

//     cout << endl;
//       }
// }


// cmake_minimum_required(VERSION 3.10)
// project(HashTable)

// # Specify the C++ standard
// set(CMAKE_CXX_STANDARD 17)
// set(CMAKE_CXX_STANDARD_REQUIRED ON)

// # Set the build type to Release for optimization
// set(CMAKE_BUILD_TYPE Release)

// # Find the benchmark library
// find_package(benchmark REQUIRED)

// # Set the sources with the correct path
// set(SOURCES ../src/bench.cpp ../src/murmurHash.cpp)

// # Define the executable
// add_executable(HashTable ${SOURCES})

// # Link the executable with benchmark and pthread libraries
// target_link_libraries(HashTable benchmark::benchmark pthread)
