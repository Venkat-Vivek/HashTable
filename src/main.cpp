#include <iostream>
#include <chrono>
#include <string>
#include "hashTable.h" 
#include <thread>

using namespace std;
using namespace std::chrono;

void benchmarkHashTable(int numElements) {
    HashTable<int, int> hashTable;
    try{
        auto startInsert = high_resolution_clock::now();
        for (int i = 1; i <= numElements; ++i) {
            hashTable.insert(i,i+1);
        }
        auto endInsert = high_resolution_clock::now();
        auto durationInsert = duration_cast<microseconds>(endInsert - startInsert);

        auto startRetrieve = high_resolution_clock::now();
        for (int i = 1; i <= numElements; ++i) {
            int ans = hashTable.get(i);
        }
        cout << endl;
        auto endRetrieve = high_resolution_clock::now();
        auto durationRetrieve = duration_cast<microseconds>(endRetrieve - startRetrieve);

        auto startUpdate = high_resolution_clock::now();
        for (int i = 1; i <= numElements; ++i) {
            bool flag = hashTable.update(i,i+numElements);
        }
        cout << endl;
        auto endUpdate = high_resolution_clock::now();
        auto durationUpdate = duration_cast<microseconds>(endUpdate - startUpdate);

        auto startDelete = high_resolution_clock::now();
        for (int i = numElements; i >= 0; --i) {
            bool flag = hashTable.remove(i);
        }
        cout << endl;
        auto endDelete = high_resolution_clock::now();
        auto durationDelete = duration_cast<microseconds>(endDelete - startDelete);

        cout << "Insertion Time for " << numElements << " elements: " 
            << durationInsert.count() * 0.001 << " ms" << endl;
        cout << "Retrieval Time for " << numElements << " elements: " 
            << durationRetrieve.count() * 0.001 << " ms" << endl;
        cout << "Update Time for " << numElements << " elements: " 
            << durationUpdate.count() * 0.001 << " ms" << endl;
        cout << "Deletion Time for " << numElements << " elements: " 
            << durationDelete.count() * 0.001 << " ms" << endl;
        hashTable.printHashTable();
    }
     catch (const exception& e) {
        cout << "Exception " << e.what() << endl;
    }
}

void test1(){
    cout << "Test 1" << endl;
    cout << "---------- " << endl;
    HashTable<int, int> hashTable;
    if(hashTable.remove(0))
        cout << "Successfully deleted" << endl;
    else 
        cout << "Could not found " << endl;
    cout << hashTable.get(0) << endl;
    if(hashTable.update(0, 1))
        cout << "Successfully updated" << endl;
    else 
        cout << "Could not found " << endl;
    hashTable.printHashTable();


    HashTable<string, int> hashTable1;
    if(hashTable1.remove(""))
        cout << "Successfully deleted" << endl;
    else 
        cout << "Could not found " << endl;
    cout << hashTable1.get("") << endl;
    if(hashTable1.update("", 1))
        cout << "Successfully updated" << endl;
    else 
        cout << "Could not found " << endl;
    hashTable1.printHashTable();

    cout << endl;
}

void test2(){
    cout << "Test 2" << endl;
    cout << "---------- " << endl;
    HashTable<int, int> hashTable;
    if(hashTable.remove(1))
        cout << "Successfully deleted" << endl;
    else 
        cout << "Could not found " << endl;
    cout << hashTable.get(1) << endl;
    if(hashTable.update(1, 2))
        cout << "Successfully updated" << endl;
    else 
        cout << "Could not found " << endl;
    hashTable.printHashTable();


    HashTable<string, int> hashTable1;
    if(hashTable1.remove("hai"))
        cout << "Successfully deleted" << endl;
    else 
        cout << "Could not found " << endl;
    cout << hashTable1.get("") << endl;
    if(hashTable1.update("hai", 1))
        cout << "Successfully updated" << endl;
    else 
        cout << "Could not found " << endl;
    hashTable1.printHashTable();

    cout << endl;
}

void test3(){
    cout << "Test 3" << endl;
    cout << "---------- " << endl;
    HashTable<int, int> hashTable;
    hashTable.insert(0, 0);

    cout << hashTable.get(0) << endl;
    if(hashTable.update(0, 2))
        cout << "Successfully updated" << endl;
    else 
        cout << "Could not found " << endl;
    if(hashTable.remove(0))
        cout << "Successfully deleted" << endl;
    else 
        cout << "Could not found " << endl;
    cout << hashTable.get(0) << endl;
    hashTable.printHashTable();

    HashTable<string, int> hashTable1;
    hashTable1.insert("", 7);
    cout << hashTable1.get("") << endl;
    if(hashTable1.update("", 1))
        cout << "Successfully updated" << endl;
    else 
        cout << "Could not found " << endl;
    if(hashTable1.remove(""))
        cout << "Successfully deleted" << endl;
    else 
        cout << "Could not found " << endl;
    hashTable1.printHashTable();

    cout << endl;
}

void test4(){
    cout << "Test 4" << endl;
    cout << "---------- " << endl;
    HashTable<int, int> hashTable;
    hashTable.insert(17, 0);

    cout << hashTable.get(17) << endl;
    if(hashTable.update(17, 19))
        cout << "Successfully updated" << endl;
    else 
        cout << "Could not found " << endl;
    cout << hashTable.get(17) << endl;
    if(hashTable.remove(17))
        cout << "Successfully deleted" << endl;
    else 
        cout << "Could not found " << endl;
    cout << hashTable.get(17) << endl;
    hashTable.printHashTable();

    HashTable<string, int> hashTable1;
    hashTable1.insert("hai", 25);
    cout << hashTable1.get("hai") << endl;
    if(hashTable1.update("hai", 8))
        cout << "Successfully updated" << endl;
    else 
        cout << "Could not found " << endl;
    if(hashTable1.remove("hai"))
        cout << "Successfully deleted" << endl;
    else 
        cout << "Could not found " << endl;
    hashTable1.printHashTable();
    cout << endl;
}

void test5(){
    cout << "Test 5" << endl;
    cout << "---------- " << endl;
    HashTable<int, int> hashTable;
    hashTable.insert(17, 8);
    hashTable.insert(17, 5);
    cout << hashTable.get(17) << endl;
    hashTable.remove(17);
    hashTable.insert(17, 25);
    cout << hashTable.get(17) << endl;
    hashTable.printHashTable();

    HashTable<string, int> hashTable1;
    hashTable1.insert("sai", 19);
    cout << hashTable1.get("sai") << endl;
    hashTable1.insert("sai", 15);
    hashTable1.remove("sai");
    cout << hashTable1.get("sai") << endl;
    hashTable1.insert("sai", 10);
    cout << hashTable1.get("sai") << endl;
    hashTable1.printHashTable();

    cout << endl;
}

void call(HashTable<int, int>& hTable){
    for(int i = 0; i < 1000; i++){
        hTable.add(5, 10);
        hTable.add1(15, 10);
    }
}

void test6(){
    cout << "Test 6" << endl;
    cout << "---------- " << endl;
    HashTable<int, int> hTable;
    hTable.insert(5,0);
    hTable.insert(15,0);

    thread t[100];
    for(int i = 0; i < 100; i++){
        t[i] = thread(call, std::ref(hTable));
    }
    for (int i = 0; i < 100; i++){
        t[i].join();
    }
    cout << "With lock: " << hTable.get(5) << endl;
    cout << "Without using locks: " << hTable.get(15) << endl;
}

int main() {
    //empty hash table & accessing updating deleting default values
    test1();
    //empty hash table & accessing updating deleting normal values 
    test2();
    //inserting default values into hash table & accessing updating deleting them
    test3();
    //inserting normal values into hash table & accessing updating deleting them
    test4();
    //inserting duplicate keys into hash table & accessing updating deleting them
    test5();
    //multithreading
    test6();
    int numElements = 10000; 
    benchmarkHashTable(numElements);
    return 0;
}
