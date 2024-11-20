#include <iostream>
#include <cstdint>
#include <string>
#include <shared_mutex>
#include <thread>
#include <mutex>
#include "murmurHash.h" 

using namespace std;

std::mutex sizedMutex; // Mutex for protecting the size variable
std::mutex deletedCountMutex; // Mutex for protecting the deletedCount variable
std::shared_mutex globalMutex; // Shared mutex for the entire table during operations

template <typename K, typename V>
struct Block {
    K key;
    V value;
    bool isDeleted;
    bool isEmpty;
    uint32_t hash;
    std::shared_mutex bucketMutex; // Mutex for each block

    Block(const K& Key = K(), const V& Value = V(), bool deleted = false, bool empty = true, uint32_t h = 0) 
        : key(Key), value(Value), isDeleted(deleted), isEmpty(empty), hash(h) {}
};

template<typename K, typename V>
struct BlockLockHashTable {
    int cap;
    int size;
    int deletedCount;
    Block<K, V>* hashTable;

    BlockLockHashTable() {
        cap = 150000;
        size = 0;
        deletedCount = 0;
        hashTable = new Block<K, V>[cap];
    }

    ~BlockLockHashTable() {
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

   
    int bucketIndex(const K& key) {
        uint32_t hashedValue = getHash(key);
        uint32_t ind = hashedValue % cap;

        {
            while (true) {
                if (hashTable[ind].isEmpty && !hashTable[ind].isDeleted) {
                    return -1; // Key not found
                }
                if (hashTable[ind].hash == hashedValue && hashTable[ind].key == key) {
                    return ind;
                }
                // Linear probing
                ind = (ind + 1) % cap;
            }
        }
    }
    
    void insertOne(K key, V value){
        uint32_t hashedValue = getHash(key);
        uint32_t ind = hashedValue % cap;

        while ((!hashTable[ind].isEmpty || hashTable[ind].isDeleted)) {
            ind = (ind + 1) % cap; // Linear probing
        }

        hashTable[ind].key = std::move(key);
        hashTable[ind].value = std::move(value);
        hashTable[ind].isDeleted = false;
        hashTable[ind].isEmpty = false;
        hashTable[ind].hash = hashedValue;
        size++;        
    }

    bool update(const K& key, V value) {
        // cout<<"update"<<endl;

        std::shared_lock<std::shared_mutex> lock_shared(globalMutex); //shared Lock the entire hash table 

        int ind = bucketIndex(key);
        if (ind != -1) {
            std::lock_guard<std::mutex> mlock(hashTable[ind].bucketMutex); // Lock the specific bucket
            hashTable[ind].value = std::move(value);
            return true;
        }
        return false;
    }

    // V get(const K& key) {
    //     // cout<<"get"<<endl;

    //     std::shared_lock<std::shared_mutex> lock_shared(globalMutex); //shared Lock the entire hash table 
    //     int ind = bucketIndex(key);
    //     if (ind != -1) {
    //         std::lock_guard<std::mutex> mlock(hashTable[ind].bucketMutex); // Lock the specific bucket
    //         return hashTable[ind].value;
    //     }
    //     return V(); // Return default value if not found
    // }

    V get(const K& key) {
        try{
            // std::shared_lock<std::shared_mutex> lock_shared(globalMutex);
            uint32_t hashedValue = getHash(key);
            uint32_t ind ;
            ind = hashedValue % cap;
            uint32_t i = 0;
            while (i < cap) {
                std::shared_lock<std::shared_mutex> mlock(hashTable[ind].bucketMutex);

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
        catch(std::exception &e){
            cout <<"error"<<endl;
            return V();
        }       
    }

    void insert(K key, V value) {
        {       
            std::lock_guard<std::mutex> sizeLock(sizedMutex);

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
                    std::unique_lock<std::shared_mutex> mlock(hashTable[(i + ind) % cap ].bucketMutex); // Lock the specific bucket

                    // std::lock_guard<std::mutex> mlock(partitionMutexes[(i + ind) % cap / partitionSize]);
                    if((!hashTable[(i + ind) % cap].isEmpty || hashTable[(i + ind) % cap].isDeleted)){

                        if (hashTable[(i + ind) % cap].hash == hashedValue && hashTable[(i + ind) % cap].key == key) {
                            return;
                        }
                    }
                    else{
                        hashTable[(i + ind) % cap].key = std::move(key);
                        hashTable[(i + ind) % cap].value = std::move(value);
                        hashTable[(i + ind) % cap].isDeleted = false;
                        hashTable[(i + ind) % cap].isEmpty = false;
                        hashTable[(i + ind) % cap].hash = hashedValue;
                        // hashTable[(i + ind) % cap] = {std::move(key), std::move(value), false, false, hashedValue};
                        {
                            std::lock_guard<std::mutex> sizeLock(sizedMutex);
                            size++;
                        }
                        return;
                    }
                }

            }
        }   
    }

    // void insert(K key, V value) {
    //             // cout<<"insert"<<endl;
    // {
    //     std::shared_lock<std::shared_mutex> lock_shared(globalMutex); // Shared lock the global mutex

    //     uint32_t hashedValue = getHash(key);
    //     uint32_t ind = hashedValue % cap;

    //     while ((!hashTable[ind].isEmpty || hashTable[ind].isDeleted)) {
    //         if (hashTable[ind].hash == hashedValue && hashTable[ind].key == key) {
    //             return; // Key already exists
    //         }
    //         ind = (ind + 1) % cap; // Linear probing
    //     }

    //     std::lock_guard<std::mutex> mlock(hashTable[ind].bucketMutex); // Lock the specific bucket

        // hashTable[ind].key = std::move(key);
        // hashTable[ind].value = std::move(value);
        // hashTable[ind].isDeleted = false;
        // hashTable[ind].isEmpty = false;
        // hashTable[ind].hash = hashedValue;

    //     {
    //         std::lock_guard<std::mutex> sizeLock(sizedMutex);
    //         size++;
    //     }
    // }
    //     // Check load factor for resizing
    //     if (size >= static_cast<int>(0.7 * cap)) {
    //         resizeTable(cap, cap * 2);
    //     }
    // }

    bool remove(const K& key) {
        bool flag = false;

        {
                    cout<<"remove"<<endl;

            std::shared_lock<std::shared_mutex> lock_shared(globalMutex); // Shared lock the entire hash table
            int ind = bucketIndex(key);

            if (ind != -1) {
                std::lock_guard<std::mutex> mlock(hashTable[ind].bucketMutex); // Lock the specific bucket

                hashTable[ind].key = K();
                hashTable[ind].value = V();
                hashTable[ind].isDeleted = true;
                hashTable[ind].isEmpty = true;
                hashTable[ind].hash = 0;
                cout<<"size"<<endl;
                {
            std::lock_guard<std::mutex> sizeLock(sizedMutex);
            size++;
            cout<<"Send"<<endl;
        }
        cout<< "delete"<< endl;
                {
            std::lock_guard<std::mutex> deleteLock(deletedCountMutex);
            deletedCount--;
            cout<<"dend"<< endl;
        }


                flag = true;
            }
        } // `lock_shared` goes out of scope and releases here

        // Check load factor and resize if necessary
        if ((size - deletedCount) < (cap / 4) && (cap / 2 >= 16)) {
            resizeTable(cap, cap / 2);
        }

        return flag;
    }

    void resizeTable(int oldCap, int newCap) {
        // Unique lock for exclusive access on global mutex during resizing
                // cout<<"resize"<<endl;

        std::unique_lock<std::shared_mutex> lock(globalMutex);
        {
          
            Block<K, V>* oldTable = hashTable;
            size = 0;
            cap = newCap;
            hashTable = new Block<K, V>[cap];
            deletedCount = 0;

            for (int i = 0; i < oldCap; i++) {
                if (!oldTable[i].isEmpty && !oldTable[i].isDeleted) {
                    insertOne(oldTable[i].key, oldTable[i].value); // Reinsert into the new table
                }
            }

            delete[] oldTable; // Clean up the old table
        }
    }

    void printHashTable() {
        for (int i = 0; i < cap; ++i) {
            if (!hashTable[i].isDeleted && !hashTable[i].isEmpty) {
                cout << "Block " << i << ": " << hashTable[i].key << " => " << hashTable[i].value << endl;
            }
        }
    }
};

