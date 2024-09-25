#include "hashTable.h"

using namespace std;

int main() {
    HashTable<string, string> h;
    string s = "programmer", ss = "coder", sss = "devp", ssss = "analyzer";
    h.insert(s, sss);
    h.insert(ss, "language");
    h.insert(sss, "react");
    h.insert(ssss, "tools");
    cout << h.update("programmer", "thinking") << endl;
    cout << h.remove("ss") << endl;
    cout << h.remove(s) << endl;
    h.printHashTable();

    HashTable<int, string> intHash;
    intHash.insert(1, "one");
    intHash.insert(2, "two");
    intHash.insert(3, "three");
    cout << "Integer Hash Table:" << endl;
    intHash.printHashTable();

    return 0;
}
