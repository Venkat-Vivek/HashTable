#include <iostream>
#include <cstdint>
#include <string>
#include <shared_mutex>
#include <thread>
#include <mutex>
#include "murmurHash.h"
using namespace std;

std::shared_mutex mt;

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

template<typename K, typename V>
struct HashTable {
    private: 
        int cap;
        int size;
        int deletedCount;
        Node<K, V>* hashTable;

    public:
        HashTable() {
            cap = 16;
            size = 0;
            deletedCount = 0;
            hashTable = new Node<K, V>[cap];
        }

        ~HashTable() {
            delete[] hashTable;
        }

    private:
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

        void resizeTable(int oldCap, int newCap) {
            Node<K, V>* oldTable = hashTable;
            size = 0;
            cap = newCap;
            hashTable = new Node<K, V>[cap];
            deletedCount = 0;
            for (int i = 0; i < oldCap; i++) {
                if (!oldTable[i].isEmpty && !oldTable[i].isDeleted) {
                    uint32_t hashedValue = getHash(oldTable[i].key);
                    uint32_t ind = hashedValue % cap;
                    
                    while ((!hashTable[ind % cap].isEmpty)) {
                        ind++;
                    }
                    hashTable[ind % cap] = {std::move(oldTable[i].key), std::move(oldTable[i].value), false, false, hashedValue};
                    
                    size++;
                }
            }
            delete[] oldTable; //clearing old table
        }

    public:
        void insert(K key, V value) {
            std::unique_lock<std::shared_mutex> lock(mt); //  lock for insert operation

            uint32_t hashedValue = getHash(key);
            uint32_t ind = hashedValue % cap;

            while ((!hashTable[ind % cap].isEmpty || hashTable[ind % cap].isDeleted)) {
                if(hashTable[ind % cap].hash == hashedValue && hashTable[ind % cap].key == key){
                    return; // Key already exists
                }
                ind++;
            }
            hashTable[ind % cap] = {std::move(key), std::move(value), false, false, hashedValue};
            
            size++;
            
            if (size >= (int)(0.7 * cap)) {
                resizeTable(cap, cap * 2);
            }
        }

        

        bool update(const K& key, V value) {
            std::unique_lock<std::shared_mutex> lock(mt); //  lock for update operation
            int ind = bucketIndex(key);
            if (ind != -1) {
                hashTable[ind].value = std::move(value);
                return true;
            }
            return false;
        }

        bool remove(const K& key) {
            bool flag = false;
            std:unique_lock<std::shared_mutex> lock(mt); //  lock for remove operation
            int ind = bucketIndex(key);
            if (ind != -1) {
                hashTable[ind] = {K(), V(), true, true, 0};
                deletedCount++;
                flag = true;
            }
        
            if ((size - deletedCount) < (cap / 4) && (cap / 2 >= 16)) {
                resizeTable(cap, cap / 2);
            }

            return flag;
        }

        V get(const K& key) {
            std::shared_lock<std::shared_mutex> lock(mt); // lock for read operation
            int ind = bucketIndex(key);
            if (ind != -1) {
                return hashTable[ind].value;
            } 
            else {
                return V();
            }
        }

        void printHashTable() {
            std::unique_lock<std::shared_mutex> lock(mt); // lock for printing hash table
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

//     // Node(const K& Key = K(), const V& Value= V(), bool deleted = false, bool empty = true, uint32_t h = 0){
//     //     key = Key;
//     //     value = Value;
//     //     isDeleted = deleted;
//     //     isEmpty = empty;
//     //     hash = h;
//     // }
//     Node(const K& Key = K(), const V& Value = V(), bool deleted = false, bool empty = true, uint32_t h = 0) 
//         : key(std::move(Key)), value(std::move(Value)), isDeleted(deleted), isEmpty(empty), hash(h) {}
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
//             // cout << "Could not found key" << endl;
//             return V();
//         }
//     }

//     void resizeTable(int oldCap, int newCap) {
//         Node<K, V>* oldTable = hashTable;
//         size = 0;
//         cap = newCap;
//         Node<K, V>*  nhashTable = new Node<K, V>[cap];
//         deletedCount = 0;
//         cout<< oldCap<< endl;
//         for (int i = 0; i < oldCap; i++) {
//             if (!oldTable[i].isEmpty && !oldTable[i].isDeleted) {
//                 insert(oldTable[i].key, oldTable[i].value);
//             }
//         }
//         hashTable = nhashTable;
//         delete[] oldTable;
//     }

//     void printHashTable() {
//         for (int i = 0; i < cap; ++i) {
//             if(!hashTable[i].isDeleted && !hashTable[i].isEmpty){
//                 cout << "Block " << i << ": " << hashTable[i].key << " => " << hashTable[i].value << endl;
//             }
//         }
//         cout << endl;
//     }
// };

//insert one

 // void insertOne(K key, V value) {
        
    //     uint32_t hashedValue = getHash(key);
    //     uint32_t ind = hashedValue % cap;
        
    //     while ((!hashTable[ind % cap].isEmpty)) {
    //         ind++;
    //     }
    //     hashTable[ind % cap] = {std::move(key), std::move(value), false, false, hashedValue};
        
    //     size++;
    // }