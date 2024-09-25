#include <iostream>
#include <cstdint>
#include <string>
#include "murmurHash.h"
using namespace std;

template<typename K, typename V>
struct Node {
    K key;
    V value;
    bool isEmpty = true;
    Node(){
        key = K();
        value = V();
    }
};

template<typename K, typename V>
struct HashTable {
    int cap;
    int size;
    Node<K, V>* hashTable;

    HashTable() {
        cap = 16;
        size = 0;
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
        uint32_t ind = murmurHash(keyPtr, len, 19) % cap;
        return ind;
    }

    void insert(const K& key, const V& value) {
        uint32_t ind = getHash(key);

        int val = 0;

        while (val < cap && !hashTable[(ind + val) % cap].isEmpty) {
            val++;
        }

        hashTable[(ind + val) % cap].key = key;
        hashTable[(ind + val) % cap].value = value;
        hashTable[(ind + val) % cap].isEmpty = false;
        size++;

        if (size >= (int)(0.7 * cap)) {
            int tempcap = cap;
            cap *= 2;
            resizeTable(tempcap);
        }
    }

    bool update(const K& key, const V& value) {
        uint32_t ind = getHash(key);

        int val = 0;
        while (val < cap) {
            if (hashTable[(ind + val) % cap].key == key) {
                hashTable[(ind + val) % cap].value = value;
                return true;
            }
            val++;
        }
        return false;
    }

    bool remove(const K& key) {
        uint32_t ind = getHash(key);

        int val = 0;
        while (val < cap) {
            if (hashTable[(ind + val) % cap].key == key) {
                hashTable[(ind + val) % cap].key = K();
                hashTable[(ind + val) % cap].value = V();
                hashTable[(ind + val) % cap].isEmpty = true;
                size--;
                if (size < (cap / 4) && (cap / 2 > 16 )) {
                    int tempcap = cap;
                    cap /= 2;
                    resizeTable(tempcap);
                }
                return true;
            }
            val++;
        }
        return false;
    }

    V get(const K& key) {
        uint32_t ind = getHash(key);

        int val = 0;
        while (val < cap) {
            if (hashTable[(ind + val) % cap].key == key) {
                return hashTable[(ind + val) % cap].value;
            }
            val++;
        }
        cout << "Could not found key" << endl;
        return V();
    }

    void resizeTable(int oldCap) {
        Node<K, V>* oldTable = hashTable;
        size = 0;
        hashTable = new Node<K, V>[cap];

        for (int i = 0; i < oldCap; i++) {
            if (!oldTable[i].isEmpty) {
                insert(oldTable[i].key, oldTable[i].value);
            }
        }
        delete[] oldTable;
    }

    void printHashTable() {
        for (int i = 0; i < cap; ++i) {
            cout << "Block " << i << ": " << hashTable[i].key << " => " << hashTable[i].value << endl;
        }
    }
};

