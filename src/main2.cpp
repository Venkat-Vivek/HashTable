// #include <benchmark/benchmark.h>
#include <iostream>
#include <chrono>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <functional>
// #include "blockLockHashTable.h" 
// #include "blockLockHashTableAtomic.h" 
// #include "hashTable.h" 
#include <exception>      
#include "partitionLockHashTable.h"

using namespace std;

HashTable<int, int> hashTable;
unordered_map<int, int> mp;
// vector<int> v(100002,0);
int numElements = 100000;
std::mutex map;

void insertion(){
    for (int i = 1; i <= numElements; ++i) {
        try{
            hashTable.insert(i, i + 1);
        }
        catch(std::exception &e){
            cout<<"insert";
        }
    }
}

void lookup(){
    for (int i = 1; i <= numElements; ++i) {
        try{
            int val=hashTable.get(i);
            // v[val]=v[val]+1;
            // cout<<"hi"<<endl;
                        std::lock_guard<std::mutex> mlock(map);

            mp[val-1]++;
        }
        catch(std::exception &e){
            cout<<"get";
        }
    }
}

// void benchmarkLookup(){
//     int numElements=100;
//     for (int i = 1; i <= numElements; ++i) {
//         try{
//         int k=hashTable.get(i);
//         // cout<<i<<"=>"<<k<<endl;
//         if(k!=i+1){ c++; cout<<i<<" "<<k<<endl;}
//         mp[k]++;               
//            }
//         catch(std::exception &e){
//             cout<<"get";
//         }
//     }
// }

// void benchmarkUpdation(){
//     int numElements=10;
//     for (int i = 1; i <= numElements; ++i) {
//         try{
//         hashTable.update(i, i + numElements);
//            }
//         catch(std::exception &e){
//             cout<<"update";
//         }
//     }
// }

// void benchmarkRemoval(){
//     int numElements=10;
//     for (int i = 1; i <= numElements; ++i) {
//         try{
//         hashTable.remove(i);
//            }
//         catch(std::exception &e){
//             cout<<"remove";
//         }
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
    }
}

// static void BENCHMARK_HASHTABLE(benchmark::State& state)
// {
//     for (auto _ : state)
//     {
//         test(16, benchmarkHashTable);
//     }
// }

int main(){
    test(8, insertion);
    test(8, lookup);
    hashTable.call();
    cout << hashTable.size << endl;

    for(auto &i: mp){
        if(i.second !=8) cout << i.first << " " << i.second << endl;
    }
    // for(auto &i:v) if (i!=4) cout <<i<<endl;

    // for(int i=2;i<=numElements+1;i++){
    //     if(mp[i]<2) cout<<i-1<<endl;
    // }
    // unordered_map<int, int> m=hashTable.mpp;

    // for(int i = 1; i <= 100000; i++){
    //     int k = hashTable.call(i);
    //     if(i != k-1) cout<<"Failure in insertion" << endl;

    // }
    // // cout << "Check" << hashTable.call(9999) << endl;

    // for(int i=1;i<=100000;i++){
    //     if (m[i]!=1) cout<<i<<endl; 
    // }
    return 0;
}
// static void INSERTION(benchmark::State& state)
// {
//     for (auto _ : state)
//     {
//         test(1, benchmarkInsertion);
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

//custom hash table
