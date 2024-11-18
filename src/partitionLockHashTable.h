#include <iostream>
#include <cstdint>
#include <string>
#include <shared_mutex>
#include <thread>
#include <mutex>    
#include <vector>
#include "murmurHash.h" 

#include<unordered_map>

using namespace std;

std::mutex sizeMutex;
std::mutex delMutex;

template <typename K, typename V>
struct Node {
    K key;
    V value;
    bool isDeleted;
    bool isEmpty;
    uint32_t hash;

    Node(const K& Key = K(), const V& Value = V(), bool deleted = false, bool empty = true, uint32_t h = 0) 
        : key(std::move(Key)), value(std::move(Value)), isDeleted(deleted), isEmpty(empty), hash(h) {}
};

template <typename K, typename V>
struct HashTable {
    int cap;
    int size;
    int deletedCount;
    mutable std::shared_mutex globalMutex;

    Node<K, V>* hashTable;
    int partitionSize;
    std::vector<std::mutex> partitionMutexes;
    unordered_map<int, int> mpp;


    HashTable(int initialCap = 16, int partSize =16) 
        : cap(initialCap), size(0), deletedCount(0), partitionSize(partSize), partitionMutexes((initialCap + partSize - 1) / partSize) {
        hashTable = new Node<K, V>[cap];
    }

    ~HashTable() {
        delete[] hashTable;
    }

    uint32_t getHash(const K& key) {
        const char* keyPtr;
        int len;
        if constexpr(std::is_same_v<K, std::string>){
            keyPtr = key.c_str();
            len = key.length();
        } else {
            keyPtr = reinterpret_cast<const char*>(&key);
            len = sizeof(K);
        }
        return murmurHash(keyPtr, len, 19);
    }


    void insertOne(K key, V value){
        uint32_t hashedValue = getHash(key);
        uint32_t ind = hashedValue % cap;

        while ((!hashTable[ind].isEmpty || hashTable[ind].isDeleted)) {
            ind = (ind + 1) % cap;
        }
        hashTable[ind % cap] = {std::move(key), std::move(value), false, false, hashedValue};
        size++;    
    }

    // int bucketIndex(const K& key) {
    //     uint32_t hashedValue = getHash(key);
    //     uint32_t ind ;
    //     ind = hashedValue % cap;
    //     int i = 0;
    //     while (i < cap) {
    //         {

    //         }
    //         if(hashTable[ind].isEmpty && !hashTable[ind].isDeleted) {
    //             return -1;
    //         }
    //         if (hashTable[ind].hash == hashedValue && hashTable[ind].key == key) {
    //             return ind;
    //         }
    //         ind = (ind + 1) % cap;
    //     }
    // }

    // V get(const K& key) {
    //     std::shared_lock<std::shared_mutex> lock_shared(globalMutex);
    //     uint32_t hashedValue = getHash(key);
    //     uint32_t ind ;
    //     ind = hashedValue % cap;
    //     uint32_t i = 0;
    //     while (i < cap) {
    //         ind = (ind + i) % cap;
    //         std::lock_guard<std::mutex> mlock(partitionMutexes[ind / partitionSize]);

    //         if(hashTable[ind].isEmpty && !hashTable[ind].isDeleted) {
    //             return -1;
    //         }
    //         if (hashTable[ind].hash == hashedValue && hashTable[ind].key == key) {
    //             return hashTable[ind].value;
    //         } 
    //         i++;
    //     }
    //     return V();
    // }

    // void call(){
    //     for(int i=0;i<cap;i++){
    //         if(!hashTable[i].isEmpty) mpp[hashTable[i].key]++;
    //      }

    //      for(int i=0;i<cap;i++){
    //         if(mpp[i]>1) cout<< i <<" "<<endl;
    //      } 
    // }

    void insert(K key, V value) {
        {       
            std::lock_guard<std::mutex> sizeLock(sizeMutex);

            if (size >= static_cast<int>(0.7 * cap)) {
                resizeTable(cap, cap * 2);
            }
        }
        {
            std::shared_lock<std::shared_mutex> lock_shared(globalMutex);

            uint32_t hashedValue = getHash(key);
            uint32_t ind = hashedValue % cap;

            for(int i=0; i < cap; i++){
                {
                    std::lock_guard<std::mutex> mlock(partitionMutexes[(i + ind) % cap / partitionSize]);
                    if((!hashTable[(i + ind) % cap].isEmpty || hashTable[(i + ind) % cap].isDeleted)){

                        if (hashTable[(i + ind) % cap].hash == hashedValue && hashTable[(i + ind) % cap].key == key) {
                            return;
                        }
                    }
                    else{
                        hashTable[(i + ind) % cap] = {std::move(key), std::move(value), false, false, hashedValue};
                        {
                            std::lock_guard<std::mutex> sizeLock(sizeMutex);
                            size++;
                        }
                        return;
                    }
                }

            }
        }
        
    }

    void resizeTable(int oldCap, int newCap) {
        std::unique_lock<std::shared_mutex> lock_shared(globalMutex);
        
        Node<K, V>* oldTable = hashTable;
        size = 0;
        cap = newCap;
        // cout<<"acquired"<<" "<<cap <<endl;
        partitionMutexes = std::vector<std::mutex>((newCap + partitionSize - 1) / partitionSize);
        hashTable = new Node<K, V>[cap];
        deletedCount = 0;

        for (int i = 0; i < oldCap; i++) {
            if (!oldTable[i].isEmpty && !oldTable[i].isDeleted) {
                insertOne(oldTable[i].key, oldTable[i].value);
            }
        }

        delete[] oldTable;
        // cout<<"complete"<<endl;
    }
};