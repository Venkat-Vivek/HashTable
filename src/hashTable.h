#include <iostream>
#include <cstdint>
#include <string>
#include <shared_mutex>
#include <thread>
#include <mutex>
#include "murmurHash.h"
using namespace std;

std::mutex mt;

template <typename K, typename V>
struct Node {
    K key;
    V value;
    bool isDeleted;
    bool isEmpty;
    uint32_t hash;

    Node(const K& Key = K(), const V& Value = V(), bool deleted = false, bool empty = true, uint32_t h = 0) {
        key = Key;
        value = Value;
        isDeleted = deleted;
        isEmpty = empty;
        hash = h;
    }
};

template<typename K, typename V>
struct HashTable {
    int cap;
    int size;
    int deletedCount;
    Node<K, V>* hashTable;

    HashTable() {
        cap = 16;
        size = 0;
        deletedCount = 0;
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
        uint32_t hashedValue = murmurHash(keyPtr, len, 19);
        return hashedValue;
    }

    void insert(K key, V value, bool flag = true) {
        
        uint32_t hashedValue = getHash(key);
        uint32_t ind = hashedValue % cap;
        if(flag){
            std::lock_guard<std::mutex> lock(mt); //  lock for insert operation

            while ((!hashTable[ind % cap].isEmpty || hashTable[ind % cap].isDeleted)) {
                if(hashTable[ind % cap].hash == hashedValue && hashTable[ind % cap].key == key){
                    return; // Key already exists
                }
                ind++;
            }
            hashTable[ind % cap] = {std::move(key), std::move(value), false, false, hashedValue};
            
            size++;
            if (size >= (int)(0.7 * cap)) {
                mt.unlock();
                resizeTable(cap, cap * 2);
            }
        }
        else{
            while ((!hashTable[ind % cap].isEmpty || hashTable[ind % cap].isDeleted)) {
                ind++;
        
            }
            hashTable[ind % cap] = {std::move(key), std::move(value), false, false, hashedValue};
            size++;
        }
    }

    int bucketIndex(const K& key) {
        uint32_t hashedValue = getHash(key);
        uint32_t ind = hashedValue % cap;
        while (!hashTable[ind].isEmpty || hashTable[ind].isDeleted) {
            if (hashTable[ind].hash == hashedValue && hashTable[ind].key == key) {
                return ind;
            }
            ind++;
            ind = ind % cap;
        }
        return -1;
    }

    bool update(const K& key, V value) {
        std::lock_guard<std::mutex> lock(mt); //  lock for update operation
        int ind = bucketIndex(key);
        if (ind != -1) {
            hashTable[ind].value = std::move(value);
            return true;
        }
        return false;
    }

    void add(const K& key, V value) {
        std::lock_guard<std::mutex> lock(mt); //  lock for add operation
        int ind = bucketIndex(key);
        if (ind != -1) {
            hashTable[ind].value += value;
        }
    }

    void add1(const K& key, V value) {
        int ind = bucketIndex(key);
        if (ind != -1) {
            hashTable[ind].value += value;
        }
    }

    bool remove(const K& key) {
        int ind = bucketIndex(key);
        std::lock_guard<std::mutex> lock(mt); //  lock for remove operation
        if (ind != -1) {
            hashTable[ind] = {K(), V(), true, true, 0};
            deletedCount++;
            if ((size - deletedCount) < (cap / 4) && (cap / 2 >= 16)) {
                mt.unlock();
                resizeTable(cap, cap / 2);
            }
            return true;
        }
        return false;
    }

    V get(const K& key) {
        std::lock_guard<std::mutex> lock(mt); // lock for read operation
        int ind = bucketIndex(key);
        if (ind != -1) {
            return hashTable[ind].value;
        } else {
            cout << "Could not find key" << endl;
            return V();
        }
    }

    void resizeTable(int oldCap, int newCap) {
        std::lock_guard<std::mutex> lock(mt); // lock for resizing
        Node<K, V>* oldTable = hashTable;
        size = 0;
        cap = newCap;
        hashTable = new Node<K, V>[cap];
        deletedCount = 0;
        for (int i = 0; i < oldCap; i++) {
            if (!oldTable[i].isEmpty && !oldTable[i].isDeleted) {
                insert(oldTable[i].key, oldTable[i].value, false);
            }
        }
        delete[] oldTable;
    }

    void printHashTable() {
        std::lock_guard<std::mutex> lock(mt); // lock for printing hash table
        for (int i = 0; i < cap; ++i) {
            if (!hashTable[i].isDeleted && !hashTable[i].isEmpty) {
                cout << "Block " << i << ": " << hashTable[i].key << " => " << hashTable[i].value << endl;
            }
        }
    }
};



// without locking

// #include <iostream>
// #include <cstdint>
// #include <string>
// #include "murmurHash.h"
// using namespace std;

// template <typename K, typename V>
// struct Node {
//     K key;
//     V value;
//     bool isDeleted;
//     bool isEmpty;
//     uint32_t hash;

//     Node(const K& Key = K(), const V& Value= V(), bool deleted = false, bool empty = true, uint32_t h = 0){
//         key = Key;
//         value = Value;
//         isDeleted = deleted;
//         isEmpty = empty;
//         hash = h;
//     }
// };

// template<typename K, typename V>
// struct HashTable {
//     int cap;
//     int size;
//     int deletedCount;
//     Node<K, V>* hashTable;

//     HashTable() {
//         cap = 16;
//         size = 0;
//         deletedCount = 0;
//         hashTable = new Node<K, V>[cap];
//     }

//     ~HashTable() {
//         delete[] hashTable;
//     }

//     uint32_t getHash(const K& key) {
//         const char* keyPtr;
//         int len;
//         if constexpr(std::is_same_v<K, std::string>){
//             keyPtr = key.c_str();
//             len = key.length();
//         } else {
//             keyPtr = reinterpret_cast<const char*>(&key);
//             len = sizeof(K);
//         }
//         uint32_t hashedValue = murmurHash(keyPtr, len, 19);
//         return hashedValue;
//     }

//     void insert(K key, V value) {
//         uint32_t hashedValue = getHash(key);
//         uint32_t ind = hashedValue % cap;

//         while ((!hashTable[ind % cap].isEmpty || hashTable[ind % cap].isDeleted)) {
//             if(hashTable[ind % cap].hash == hashedValue && hashTable[ind % cap].key == key){
//                 return;
//             }
//             ind++;
//         }

//         hashTable[ind % cap] = {std::move(key), std::move(value) , false, false, hashedValue};
//         size++;
   
//         if (size >= (int)(0.7 * cap)) {
//             resizeTable(cap, cap*2);
//         }
//     }

//     int bucketIndex(const K& key){
//         uint32_t hashedValue = getHash(key);
//         uint32_t ind = hashedValue % cap;
//         while(!hashTable[ind].isEmpty || hashTable[ind].isDeleted){
//             if (hashTable[ind].hash == hashedValue && hashTable[ind].key == key) {
//                 return ind;
//             }
//             ind++;
//             ind = ind % cap;
//         }
//         return -1;
//     }

//     bool update(const K& key, V value) {
//         int ind = bucketIndex(key);
//         if(ind != -1){
//             hashTable[ind].value = std::move(value);
//             return true;
//         }
//         else{
//             return false;
//         }
//     }

//     bool remove(const K& key) {
//         int ind = bucketIndex(key);
//         if(ind != -1){
//             hashTable[ind] = {K(), V(), true, true, 0};
//             deletedCount++;
//             if ((size - deletedCount) < (cap / 4) && (cap / 2 >= 16 )) {
//                 resizeTable(cap, cap / 2);
//             }
//             return true;
//         }
//         else{
//             return false;
//         }
//     }

//     V get(const K& key) {
//         int ind = bucketIndex(key);
//         if(ind != -1){
//             V ans = hashTable[ind].value;
//             return ans;
//         }
//         else{
//             cout << "Could not found key" << endl;
//             return V();
//         }
//     }

//     void resizeTable(int oldCap, int newCap) {
//         Node<K, V>* oldTable = hashTable;
//         size = 0;
//         cap = newCap;
//         hashTable = new Node<K, V>[cap];
//         deletedCount = 0;
//         for (int i = 0; i < oldCap; i++) {
//             if (!oldTable[i].isEmpty && !oldTable[i].isDeleted) {
//                 insert(oldTable[i].key, oldTable[i].value);
//             }
//         }
//         delete[] oldTable;
//     }

//     void printHashTable() {
//         for (int i = 0; i < cap; ++i) {
//             if(!hashTable[i].isDeleted && !hashTable[i].isEmpty){
//                 cout << "Block " << i << ": " << hashTable[i].key << " => " << hashTable[i].value << endl;
//             }
//         }
//     }
// };