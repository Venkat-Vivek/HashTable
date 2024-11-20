#include <iostream>
#include <cstdint>
#include <string>
#include <shared_mutex>
#include <thread>
#include <mutex>    
#include <vector>
#include "murmurHash.h" 
#include <exception>      

#include<unordered_map>

using namespace std;

std::mutex sizeMutex;
std::mutex delMutex;
std::mutex capMutex;

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
    std::vector<std::shared_mutex> partitionMutexes;
    unordered_map<int, int> mpp;


    HashTable(int initialCap = 150000, int partSize = 256) 
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

    int bucketIndex(const K& key) {
        uint32_t hashedValue = getHash(key);
        uint32_t ind ;
        ind = hashedValue % cap;
        int i = 0;
        while (i < cap) {
            {

            }
            if(hashTable[ind].isEmpty && !hashTable[ind].isDeleted) {
                return -1;
            }
            if (hashTable[ind].hash == hashedValue && hashTable[ind].key == key) {
                return ind;
            }
            ind = (ind + 1) % cap;
        }
    }

    V get(const K& key) {
        // std::shared_lock<std::shared_mutex> lock_shared(globalMutex);
        uint32_t hashedValue = getHash(key);
        uint32_t ind ;
        ind = hashedValue % cap;
        uint32_t i = 0;
        while (i < cap) {
            std::shared_lock<std::shared_mutex> mlock(partitionMutexes[ind / partitionSize]);

            if(hashTable[ind].isEmpty && !hashTable[ind].isDeleted) {
                // cout << key << endl;
                return V();
            }
            if (hashTable[ind].hash == hashedValue && hashTable[ind].key == key) {
                // cout << key << " " << hashTable[ind].value << endl;
                return hashTable[ind].value;
            } 
            i++;
            ind = (ind + 1) % cap;
        }
        return V();
    }

    // V call(const K& key) {
    //     uint32_t ind = bucketIndex(key);
    //     return hashTable[ind].value;
    // }

    void call(){
        for(int i=0;i<cap;i++){
            if(!hashTable[i].isEmpty) mpp[hashTable[i].key]++;
         }

         for(int i=1;i<=size;i++){
            if(mpp[i]!=1) cout<< i <<" "<<endl;
         }
         
    }

    void insert(K key, V value) {
        {       
            std::lock_guard<std::mutex> sizeLock(sizeMutex);

            if (size >= static_cast<int>(0.7 * cap)) {
                resizeTable(cap, cap * 2);
            }
        }
        {
            // std::shared_lock<std::shared_mutex> lock_shared(globalMutex);
            // std::lock_guard<std::mutex> sizeLock(capMutex);

            uint32_t hashedValue = getHash(key);
            uint32_t ind = hashedValue % cap;

            for(int i=0; i < cap; i++){
                {
                    std::unique_lock<std::shared_mutex> mlock(partitionMutexes[(i + ind) % cap / partitionSize]);
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
        // std::unique_lock<std::shared_mutex> lock_shared(globalMutex);
        
        Node<K, V>* oldTable = hashTable;
        size = 0;
        cap = newCap;
        // cout<<"acquired"<<" "<<cap <<endl;
        partitionMutexes = std::vector<std::shared_mutex>((newCap + partitionSize - 1) / partitionSize);
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


    // bool update(const K& key, V value) {
    //     std::shared_lock<std::shared_mutex> lock_shared(globalMutex);

    //     int ind = bucketIndex(key);
    //     if (ind != -1) {
    //         std::lock_guard<std::mutex> mlock(partitionMutexes[ind / partitionSize]);
    //         hashTable[ind].value = std::move(value);
    //         return true;
    //     }
    //     return false;
    // }


    // bool remove(const K& key) {
    //     bool flag = false;

    //     {
    //         std::shared_lock<std::shared_mutex> lock_shared(globalMutex);
    //         int ind = bucketIndex(key);

    //         if (ind != -1) {
    //             std::lock_guard<std::mutex> mlock(partitionMutexes[ind / partitionSize]);

    //             hashTable[ind].key = K();
    //             hashTable[ind].value = V();
    //             hashTable[ind].isDeleted = true;
    //             hashTable[ind].isEmpty = true;
    //             hashTable[ind].hash = 0;

    //             {
    //                 std::lock_guard<std::mutex> sizeLock(sizeMutex);
    //                 size--;
    //             }
    //             {
    //                 std::lock_guard<std::mutex> delLock(delMutex);
    //                 deletedCount++;
    //             }
    //             flag = true;
    //         }
    //     }

    //     if ((size - deletedCount) < (cap / 4) && (cap / 2 >= 16)) {
    //         resizeTable(cap, cap / 2);
    //     }

    //     return flag;
    // }



  
// #include <iostream>
// #include <cstdint>
// #include <string>
// #include <shared_mutex>
// #include <thread>
// #include <mutex>
// #include "murmurHash.h" 

// using namespace std;
// std::mutex sizeMutex; // Mutex for protecting the size variable
// std::mutex delMutex; // Mutex for protecting the size variable

// std::shared_mutex globalMutex; // Shared mutex for the entire table during operations

// template <typename K, typename V>
// struct Node {
//     K key;
//     V value;
//     bool isDeleted;
//     bool isEmpty;
//     uint32_t hash;

//     Node(const K& Key = K(), const V& Value = V(), bool deleted = false, bool empty = true, uint32_t h = 0) 
//         : key(std::move(Key)), value(std::move(Value)), isDeleted(deleted), isEmpty(empty), hash(h) {}
// };

// template <typename K, typename V>
// struct HashTable {
//     int cap;
//     int size;
//     int deletedCount;
//     Node<K, V>* hashTable;
//     int partitionSize;
//     std::vector<std::mutex> partitionMutexes;

//     HashTable(int initialCap = 16, int partSize = 16) 
//         : cap(initialCap), size(0), deletedCount(0), partitionSize(partSize), partitionMutexes((initialCap + partSize - 1) / partSize) {
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
//         return murmurHash(keyPtr, len, 19);
//     }

   
//     int bucketIndex(const K& key) {
//         uint32_t hashedValue = getHash(key);
//         uint32_t ind = hashedValue % cap;

//         {
//             while (true) {
//                 if (hashTable[ind].isEmpty && !hashTable[ind].isDeleted) {
//                     return -1; // Key not found
//                 }
//                 if (hashTable[ind].hash == hashedValue && hashTable[ind].key == key) {
//                     return ind;
//                 }
//                 // Linear probing
//                 ind = (ind + 1) % cap;
//             }
//         }
//     }

//     void insertOne(K key, V value){
//         uint32_t hashedValue = getHash(key);
//         uint32_t ind = hashedValue % cap;

//         while ((!hashTable[ind].isEmpty || hashTable[ind].isDeleted)) {
//             ind = (ind + 1) % cap; // Linear probing
//         }

//         hashTable[ind].key = std::move(key);
//         hashTable[ind].value = std::move(value);
//         hashTable[ind].isDeleted = false;
//         hashTable[ind].isEmpty = false;
//         hashTable[ind].hash = hashedValue;
//         size++;        
//     }

//     bool update(const K& key, V value) {
//         // cout<<"update"<<endl;

//         std::shared_lock<std::shared_mutex> lock_shared(globalMutex); //shared Lock the entire hash table 

//         int ind = bucketIndex(key);
//         if (ind != -1) {
//             std::lock_guard<std::mutex> mlock(partitionMutexes[ind / partitionSize]); // Lock the specific bucket
//             hashTable[ind].value = std::move(value);
//             return true;
//         }
//         return false;
//     }

  
// void insert(K key, V value) {
//     {
//     std::shared_lock<std::shared_mutex> lock_shared(globalMutex); // Shared lock the global mutex

//     uint32_t hashedValue = getHash(key);
//     uint32_t ind = hashedValue % cap;

//     while ((!hashTable[ind].isEmpty || hashTable[ind].isDeleted)) {
//         if (hashTable[ind].hash == hashedValue && hashTable[ind].key == key) {
//             return; // Key already exists
//         }
//         ind = (ind + 1) % cap; // Linear probing
//     }

//             std::lock_guard<std::mutex> mlock(partitionMutexes[ind / partitionSize]); // Lock the specific bucket

//     hashTable[ind].key = std::move(key);
//     hashTable[ind].value = std::move(value);
//     hashTable[ind].isDeleted = false;
//     hashTable[ind].isEmpty = false;
//     hashTable[ind].hash = hashedValue;

//     // Lock the size mutex to modify `size` safely
//     {
//         std::lock_guard<std::mutex> sizeLock(sizeMutex);
//         size++;
//     }
// }

//     // Check load factor for resizing
//     if (size >= static_cast<int>(0.7 * cap)) {
//         resizeTable(cap, cap * 2);
//     }
// }

// bool remove(const K& key) {
//     bool flag = false;

//     {
//         std::shared_lock<std::shared_mutex> lock_shared(globalMutex); // Shared lock the entire hash table
//         int ind = bucketIndex(key);

//         if (ind != -1) {
//             std::lock_guard<std::mutex> mlock(partitionMutexes[ind / partitionSize]); // Lock the specific bucket

//             hashTable[ind].key = K();
//             hashTable[ind].value = V();
//             hashTable[ind].isDeleted = true;
//             hashTable[ind].isEmpty = true;
//             hashTable[ind].hash = 0;

//             // Lock the size mutex to modify `size` safely
//             {
//                 std::lock_guard<std::mutex> sizeLock(sizeMutex);
//                 size--;
//             }
//             {
//                 std::lock_guard<std::mutex> delLock(delMutex);
//                 deletedCount++;

//             }
//             flag = true;
//         }
//     } // `lock_shared` goes out of scope and releases here

//     // Check load factor and resize if necessary
//     if ((size - deletedCount) < (cap / 4) && (cap / 2 >= 16)) {
//         resizeTable(cap, cap / 2);
//     }

//     return flag;
// }




//     V get(const K& key) {
//         // cout<<"get"<<endl;

//         std::shared_lock<std::shared_mutex> lock_shared(globalMutex); //shared Lock the entire hash table 
//         int ind = bucketIndex(key);
//         if (ind != -1) {
//             std::lock_guard<std::mutex> mlock(partitionMutexes[ind / partitionSize]); // Lock the specific bucket
//             return hashTable[ind].value;
//         }
//         return V(); // Return default value if not found
//     }

// void resizeTable(int oldCap, int newCap) {
//     // Unique lock for exclusive access on global mutex during resizing
//     std::unique_lock<std::shared_mutex> lock(globalMutex);

//     Node<K, V>* oldTable = hashTable;
//     size = 0;  // Reset size safely, as we have exclusive access
//     cap = newCap;
//     hashTable = new Node<K, V>[cap];
//     deletedCount = 0;

//     for (int i = 0; i < oldCap; i++) {
//         if (!oldTable[i].isEmpty && !oldTable[i].isDeleted) {
//             insertOne(oldTable[i].key, oldTable[i].value); // Reinsert into the new table
//         }
//     }

//     delete[] oldTable; // Clean up the old table
// }

//     // void printHashTable() {
//     //     for (int i = 0; i < cap; ++i) {
//     //         if (!hashTable[i].isDeleted && !hashTable[i].isEmpty) {
//     //             cout << "Block " << i << ": " << hashTable[i].key << " => " << hashTable[i].value << endl;
//     //         }
//     //     }
//     // }
// };

