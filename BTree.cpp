//
//  BTree.cpp
//  Simple Database
//
//  Created by Kevin on 16/7/3.
//

#include <stdio.h>
#include <iostream>
#include "BTree.h"
using namespace std;


FILEPOS BTree::newNode(){
    if (!idxremoved.empty()){
        FILEPOS address = idxremoved.front();
        idxremoved.pop();
        return address;
    }
    else{
        fseek(idxfile, 0, SEEK_END);
        return ftell(idxfile);
    }
}

void BTree::readNode(const FILEPOS address, const BTnode &r){
    fseek(idxfile, address, SEEK_SET);
    fread((char*)(&r), sizeof(BTnode), 1, idxfile);
}

void BTree::writeNode(const FILEPOS address, const BTnode &r){
    fseek(idxfile, address, SEEK_SET);
    fwrite((char*)(&r), sizeof(BTnode), 1, idxfile);
}

FILEPOS BTree::getAddress(){
    if (!datremoved.empty()){
        FILEPOS address = datremoved.front();
        datremoved.pop();
        return address;
    }
    else{
        fseek(datfile, 0, SEEK_END);
        return ftell(datfile);
    }
}

DATATYPE BTree::readData(const FILEPOS address){
    DATATYPE data;
    fseek(datfile, address, SEEK_SET);
    fread((char*)(&data), sizeof(DATATYPE), 1, datfile);
    return data;
}

void BTree::writeData(const FILEPOS address, const DATATYPE data){
    fseek(datfile, address, SEEK_SET);
    fwrite((char*)(&data), sizeof(DATATYPE), 1, datfile);
}

void BTree::changeRoot(const FILEPOS root){
    fseek(rootfile, 0, SEEK_SET);
    fwrite((char*)(&root), sizeof(FILEPOS), 1, rootfile);
}

void BTree::buildTree(){
    ROOT = newNode();
    changeRoot(ROOT);
    BTnode r;
    r.keynum = 0;
    for (int i = 0; i < MAX_KEY + 1; ++i)
        r.pointer[i] = -1;
    r.isLeaf = true;
    writeNode(ROOT, r);
}

void BTree::splitNode(BTnode &r, BTnode &parent, int childNum){
    int half = MAX_KEY / 2;
    for (int i = parent.keynum; i > childNum; --i){
        parent.key[i] = parent.key[i-1];
        parent.pointer[i] = parent.pointer[i-1];
    }
    parent.keynum++;
    BTnode t;
    FILEPOS address = newNode();
    parent.key[childNum] = r.key[half];
    parent.pointer[childNum] = address;
    for (int i = half; i < MAX_KEY; ++i){
        t.key[i-half] = r.key[i];
        t.pointer[i-half] = r.pointer[i];
    }
    t.keynum = MAX_KEY - half;
    r.keynum = half;
    t.isLeaf = r.isLeaf;
    if (r.isLeaf){
        t.pointer[MAX_KEY] = r.pointer[MAX_KEY];
        r.pointer[MAX_KEY] = address;
    }
    writeNode(address, t);
}

void BTree::splitRoot(BTnode &root){
    BTnode newRoot;
    BTnode rc;
    int half = MAX_KEY / 2;
    FILEPOS address = newNode();
    newRoot.isLeaf = false;
    newRoot.keynum = 0;
    newRoot.pointer[0] = ROOT;
    newRoot.pointer[1] = address;
    newRoot.key[0] = root.key[0];
    newRoot.key[1] = root.key[half];
    newRoot.keynum += 2;
    for (int i = half; i < MAX_KEY; ++i){
        rc.key[i-half] = root.key[i];
        rc.pointer[i-half] = root.pointer[i];
    }
    rc.keynum = MAX_KEY - half;
    root.keynum = half;
    rc.isLeaf = root.isLeaf;
    if (root.isLeaf){
        root.pointer[MAX_KEY] = address;
        rc.pointer[MAX_KEY] = -1;
    }
    writeNode(address, rc);
    writeNode(ROOT, root);
    ROOT = newNode();
    changeRoot(ROOT);
    writeNode(ROOT, newRoot);
}

void BTree::mergeNode(BTnode &parent, BTnode &left, BTnode &right, int leftnum){
    for (int i = 0; i < right.keynum; ++i){
        left.key[left.keynum] = right.key[i];
        left.pointer[left.keynum] = right.pointer[i];
        left.keynum++;
    }
    right.keynum = 0;
    idxremoved.push(parent.pointer[leftnum+1]);
    for (int i = leftnum+1; i < parent.keynum-1; ++i){
        parent.key[i] = parent.key[i+1];
        parent.pointer[i] = parent.pointer[i+1];
    }
    parent.keynum--;
    if (parent.key[leftnum] != left.key[0]) parent.key[leftnum] = left.key[0];
    if (left.isLeaf) left.pointer[MAX_KEY] = right.pointer[MAX_KEY];
    writeNode(parent.pointer[leftnum], left);
}

void BTree::giveNode(BTnode &parent, BTnode &left, BTnode &right, int leftnum){
    if (left.keynum < right.keynum){
        left.key[left.keynum] = right.key[0];
        left.pointer[left.keynum] = right.pointer[0];
        left.keynum++;
        parent.key[leftnum+1] = right.key[1];
        for(int i = 0; i < MAX_KEY-1; i++){
            right.key[i] = right.key[i+1];
            right.pointer[i] = right.pointer[i+1];
        }
        right.keynum--;
    }
    else{
        right.key[right.keynum] = left.key[left.keynum-1];
        right.pointer[right.keynum] = left.pointer[left.keynum-1];
        right.keynum++;
        left.keynum--;
    }
    writeNode(parent.pointer[leftnum], left);
    writeNode(parent.pointer[leftnum+1], right);
}

void BTree::insertKey(FILEPOS position, KVpair &data){
    BTnode r;
    readNode(position, r);
    int i;
    for (i = 0; i < r.keynum && r.key[i] <= data.key; ++i);
    if (r.isLeaf){
        for (int j = r.keynum; j > i; j--){
            r.key[j] = r.key[j-1];
            r.pointer[j] = r.pointer[j-1];
        }
        r.key[i] = data.key;
        r.pointer[i] = getAddress();
        r.keynum++;
        writeNode(position, r);
        writeData(r.pointer[i], data.data);
    }
    else{
        if (i == 0){
            r.key[0] = data.key;
            writeNode(position, r);
            insertKey(r.pointer[0], data);
            BTnode rc;
            readNode(r.pointer[0], rc);
            if (rc.keynum == MAX_KEY){
                splitNode(rc, r, 1);
                writeNode(position, r);
                writeNode(r.pointer[0], rc);
            }
        }
        else{
            insertKey(r.pointer[i-1], data);
            BTnode rc;
            readNode(r.pointer[i-1], rc);
            if (rc.keynum == MAX_KEY){
                splitNode(rc, r, i);
                writeNode(position, r);
                writeNode(r.pointer[i-1], rc);
            }
        }
    }
    if (position == ROOT && r.keynum == MAX_KEY){
        splitRoot(r);
    }
}

DATATYPE BTree::findData(const KEYTYPE key){
    BTnode r;
    BTnode temp;
    readNode(ROOT, r);
    DATATYPE data = "";
    while (!r.isLeaf){
        if (r.key[0] > key || r.keynum > MAX_KEY) return "";
        for (int i = 0; i < r.keynum; ++i){
            if ((r.key[i] <= key && key < r.key[i+1]) || (r.key[i] <= key && i == r.keynum-1)){
                readNode(r.pointer[i], temp);
                r = temp;
                break;
            }
        }
    }
    for (int i = 0; i < r.keynum; ++i){
        if (r.key[i] == key){
            FILEPOS address = r.pointer[i];
            data = readData(address);
        }
    }
    return data;
}

void BTree::replaceData(const KEYTYPE key, const DATATYPE data){
    BTnode r;
    BTnode temp;
    readNode(ROOT, r);
    FILEPOS address = 0;
    while (!r.isLeaf){
        for (int i = 0; i < r.keynum; ++i){
            if ((r.key[i] <= key && key < r.key[i+1]) || (r.key[i] <= key && i == r.keynum-1)){
                readNode(r.pointer[i], temp);
                r = temp;
                break;
            }
        }
    }
    for (int i = 0; i < r.keynum; ++i){
        if (r.key[i] == key){
            address = r.pointer[i];
        }
    }
    writeData(address, data);
}

void BTree::deleteKey(FILEPOS address, const KEYTYPE key){
    BTnode r;
    readNode(address, r);
    FILEPOS fileadd = 0;
    if (key < r.key[0]) return;
    if (r.isLeaf){
        if (r.keynum == 0) return;
        for (int i = 0; i < r.keynum; ++i){
            if (r.key[i] == key){
                fileadd = r.pointer[i];
                datremoved.push(fileadd);
                for (int j = i; j < r.keynum-1; ++j){
                    r.key[j] = r.key[j+1];
                    r.pointer[j] = r.pointer[j+1];
                }
                r.keynum--;
                writeNode(address, r);
                break;
            }
        }
    }
    else{
        int i;
        for (i = 0; i < r.keynum && r.key[i] <= key; ++i);
        deleteKey(r.pointer[i-1], key);
        BTnode rc;
        readNode(r.pointer[i-1], rc);
        if (rc.keynum == 0){
            idxremoved.push(r.pointer[i-1]);
            for (int j = i-1; j < r.keynum-1; ++j){
                r.key[j] = r.key[j+1];
                r.pointer[j] = r.pointer[j+1];
            }
            r.keynum--;
            writeNode(address, r);
            if (r.keynum == 0) r.isLeaf = true;
        }
        else if (key == r.key[i-1]){
            r.key[i-1] = rc.key[0];
            writeNode(address, r);
        }
        else if (rc.keynum < MAX_KEY/2 && i > 1){
            BTnode left;
            readNode(r.pointer[i-2], left);
            if (left.keynum < MAX_KEY/2)  {
                mergeNode(r, left, rc, i-2);
                writeNode(address, r);
            }
            else if (left.keynum > MAX_KEY/2){
                giveNode(r, left ,rc, i-2);
                writeNode(address, r);
            }
        }
        else if (rc.keynum < MAX_KEY/2 && i < r.keynum){
            BTnode right;
            readNode(r.pointer[i], right);
            if (right.keynum < MAX_KEY/2){
                mergeNode(r, rc, right, i-1);
                writeNode(address, r);
            }
            else if (right.keynum > MAX_KEY/2){
                giveNode(r, rc, right, i-1);
                writeNode(address, r);
            }
        }
    }
}

void BTree::getAllKey(vector<KEYTYPE> &keys){
    BTnode r;
    readNode(ROOT, r);
    while (!r.isLeaf){
        readNode(r.pointer[0], r);
    }
    if (r.keynum != 0){
        for (int i = 0; i < r.keynum; ++i){
            KEYTYPE key = r.key[i];
            keys.push_back(key);
        }
    }
    if (r.pointer[MAX_KEY] == -1) return;
    do{
        readNode(r.pointer[MAX_KEY], r);
        for (int i = 0; i < r.keynum; ++i){
            KEYTYPE key = r.key[i];
            keys.push_back(key);
        }
    } while (r.pointer[MAX_KEY] != -1);
}

void BTree::DB_open(){
    idxfile = fopen("/Users/kevinjyx/Documents/XCode/Simple Database/Simple Database/simdb.idx", "rb+");
    datfile = fopen("/Users/kevinjyx/Documents/XCode/Simple Database/Simple Database/simdb.dat", "rb+");
    rootfile = fopen("/Users/kevinjyx/Documents/XCode/Simple Database/Simple Database/simdb.rot", "rb+");
    fseek(rootfile, 0, SEEK_SET);
    fread((char*)(&ROOT), sizeof(FILEPOS), 1, rootfile);
    ridx = fopen("/Users/kevinjyx/Documents/XCode/Simple Database/Simple Database/reidx.bin", "rb+");
    rdat = fopen("/Users/kevinjyx/Documents/XCode/Simple Database/Simple Database/redat.bin", "rb+");
    while (feof(ridx) != 0){
        FILEPOS x;
        fread((char*)(&x), sizeof(FILEPOS), 1, ridx);
        idxremoved.push(x);
        fseek(ridx, sizeof(FILEPOS), SEEK_CUR);
    }
    while (feof(rdat) != 0){
        FILEPOS x;
        fread((char*)(&x), sizeof(FILEPOS), 1, rdat);
        datremoved.push(x);
        fseek(rdat, sizeof(FILEPOS), SEEK_CUR);
    }
}

void BTree::DB_wopen(){
    idxfile = fopen("/Users/kevinjyx/Documents/XCode/Simple Database/Simple Database/simdb.idx", "wb+");
    datfile = fopen("/Users/kevinjyx/Documents/XCode/Simple Database/Simple Database/simdb.dat", "wb+");
    rootfile = fopen("/Users/kevinjyx/Documents/XCode/Simple Database/Simple Database/simdb.rot", "wb+");
    fseek(rootfile, 0, SEEK_SET);
    fread((char*)(&ROOT), sizeof(FILEPOS), 1, rootfile);
    ridx = fopen("/Users/kevinjyx/Documents/XCode/Simple Database/Simple Database/reidx.bin", "rb+");
    rdat = fopen("/Users/kevinjyx/Documents/XCode/Simple Database/Simple Database/redat.bin", "rb+");
    while (feof(ridx) != 0){
        FILEPOS x;
        fread((char*)(&x), sizeof(FILEPOS), 1, ridx);
        idxremoved.push(x);
        fseek(ridx, sizeof(FILEPOS), SEEK_CUR);
    }
    while (feof(rdat) != 0){
        FILEPOS x;
        fread((char*)(&x), sizeof(FILEPOS), 1, rdat);
        datremoved.push(x);
        fseek(rdat, sizeof(FILEPOS), SEEK_CUR);
    }
}

void BTree::DB_close(){
    fclose(idxfile);
    fclose(datfile);
    fclose(rootfile);
    ridx = fopen("/Users/kevinjyx/Documents/XCode/Simple Database/Simple Database/reidx.bin", "wb+");
    rdat = fopen("/Users/kevinjyx/Documents/XCode/Simple Database/Simple Database/redat.bin", "wb+");
    while (!idxremoved.empty()){
        FILEPOS x = idxremoved.front();
        idxremoved.pop();
        fwrite((char*)(&x), sizeof(FILEPOS), 1, ridx);
        fseek(ridx, sizeof(FILEPOS), SEEK_CUR);
    }
    while (!datremoved.empty()){
        FILEPOS x = datremoved.front();
        datremoved.pop();
        fwrite((char*)(&x), sizeof(FILEPOS), 1, rdat);
        fseek(rdat, sizeof(FILEPOS), SEEK_CUR);
    }
    fclose(ridx);
    fclose(rdat);
}

void BTree::DB_store(FILEPOS position, KVpair &data){
    if (findData(data.key) == "")
        insertKey(position, data);
    else  replaceData(data.key, data.data);
}

DATATYPE BTree::DB_fetch(const KEYTYPE key){
    DATATYPE data = findData(key);
    return data;
}

void BTree::DB_delete(const KEYTYPE key){
    deleteKey(ROOT, key);
}

void BTree::DB_nextrec(){
    BTnode r;
    readNode(ROOT, r);
    while (!r.isLeaf){
        readNode(r.pointer[0], r);
    }
    if (r.keynum != 0){
        for (int i = 0; i < r.keynum; ++i){
            KEYTYPE key = r.key[i];
            DATATYPE data = readData(r.pointer[i]);
            cout << key << "   " << data << endl;
        }
    }
    if (r.pointer[MAX_KEY] == -1) return;
    do{
        readNode(r.pointer[MAX_KEY], r);
        for (int i = 0; i < r.keynum; ++i){
            KEYTYPE key = r.key[i];
            DATATYPE data = readData(r.pointer[i]);
            cout << key << "   " << data << endl;
        }
    } while (r.pointer[MAX_KEY] != -1);
}
