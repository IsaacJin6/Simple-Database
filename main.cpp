//
//  main.cpp
//  Simple Database
//
//  Created by Kevin on 16/7/3.
//

#include <iostream>
#include <ctime>
#include "stdlib.h"
#include "BTree.h"
using namespace std;

#define NREC 1000000

void test(BTree Database){
    clock_t start,end;
    start = clock();
    Database.DB_wopen();
    Database.buildTree();
    vector<KEYTYPE> randomKey;
    KVpair data;
    srand((unsigned)time(0));
    for (long i = 0; i < NREC; i++){
        data.key = rand();
        randomKey.push_back(data.key);
        data.data = rand();
        Database.DB_store(Database.ROOT, data);
    }
    int j = 1;
    while (j < NREC * 5) {
        if (j % 37 == 0){
            long long key = randomKey[rand() % randomKey.size()];
            randomKey.erase(randomKey.begin()+rand() % randomKey.size());
            Database.DB_delete(key);
        }
        if (j % 11 == 0){
            data.key = rand();
            randomKey.push_back(data.key);
            data.data = rand();
            Database.DB_store(Database.ROOT, data);
            Database.DB_fetch(data.key);
        }
        if (j % 17 == 0){
            long long key = randomKey[rand() % randomKey.size()];
            data.key = key;
            data.data = rand();
            Database.DB_store(Database.ROOT, data);
        }
        j++;
    }
    vector<KEYTYPE> keys;
    Database.getAllKey(keys);
    for (int i = 0; i < keys.size(); ++i){
        Database.DB_delete(keys[i]);
        int k = 0;
        while (k < 10){
            KEYTYPE key = rand();
            Database.DB_fetch(key);
            k++;
        }
    }
    end = clock();
    cout << "Time using: " << (end-start)/CLOCKS_PER_SEC << " s." << endl;
}


int main(int argc, const char * argv[]) {
    BTree Database;
    cout << "Do you want to do the test?(y/n): ";
    string str;
    cin >> str;
    if (str == "y"){
        test(Database);
        cout << "Test complete." << endl;
        return 0;
    }
    Database.DB_open();
    cout << "Welcome to the simple database!" << endl;
    cout << "1.Store    2.Find    3.Delete    4.Clear    5.Traverse" << endl;
    while (true){
        cout << "What do you want to do? ";
        int n;
        cin >> n;
        if (n == 1){
            cout << "Please input the key and the value: ";
            KEYTYPE key;
            DATATYPE data;
            KVpair pair;
            cin >> key >> data;
            pair.key = key;
            pair.data = data;
            Database.DB_store(Database.ROOT, pair);
            cout << "Store complete." << endl;
        }
        else if (n == 2){
            cout << "Please input the key: ";
            KEYTYPE key;
            cin >> key;
            DATATYPE data = Database.DB_fetch(key);
            if (data == "") cout << "Not found" << endl;
            else cout << data <<endl;
        }
        else if (n == 3){
            cout << "Please input the key: ";
            KEYTYPE key;
            cin >> key;
            Database.DB_delete(key);
            cout << "Delete complete." << endl;
        }
        else if (n == 4){
            Database.DB_close();
            Database.DB_wopen();
            Database.buildTree();
            cout << "Clear complete." << endl;
        }
        else if (n == 5){
            Database.DB_nextrec();
            cout << "Traverse complete." << endl;
        }
        else break;
    }
    Database.DB_close();
    return 0;
}
