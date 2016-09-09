//
//  BTree.h
//  Simple Database
//
//  Created by Kevin on 16/7/3.
//

#ifndef BTree_h
#define BTree_h

#include <string>
#include <queue>
#include <vector>

typedef long long KEYTYPE;
typedef std::string DATATYPE;
typedef long long FILEPOS;

#define MAX_KEY 400

struct KVpair{
    KEYTYPE key;
    DATATYPE data;
};

struct BTnode{
    KEYTYPE key[MAX_KEY];                  // store keys
    FILEPOS pointer[MAX_KEY + 1];          // store pointers
    int keynum;                            // the number of keys
    bool isLeaf;                           // whether the node is a leaf node
};

class BTree{
    FILE* idxfile;
    FILE* datfile;
    FILE* rootfile;
    FILE* ridx;
    FILE* rdat;
    std::queue<FILEPOS> datremoved;
    std::queue<FILEPOS> idxremoved;
    FILEPOS newNode();
    void readNode(const FILEPOS address, const BTnode &r);
    void writeNode(const FILEPOS address, const BTnode &r);
    void splitNode(BTnode &r, BTnode &parent, int childNum);
    void splitRoot(BTnode &root);
    void mergeNode(BTnode &parent, BTnode &left, BTnode &right, int leftnum);
    void giveNode(BTnode &parent, BTnode &left, BTnode &right, int leftnum);
    void insertKey(FILEPOS position, KVpair &data);
    DATATYPE readData(const FILEPOS address);
    void writeData(const FILEPOS address, const DATATYPE data);
    FILEPOS getAddress();
    void changeRoot(const FILEPOS root);
    void replaceData(const KEYTYPE key, const DATATYPE data);
    DATATYPE findData(const KEYTYPE key);
    void deleteKey(FILEPOS address, const KEYTYPE key);
public:
    FILEPOS ROOT;
    void getAllKey(std::vector<KEYTYPE> &keys);
    void buildTree();
    void DB_wopen();
    void DB_open();
    void DB_close();
    void DB_store(FILEPOS position, KVpair &data);
    DATATYPE DB_fetch(const KEYTYPE key);
    void DB_delete(const KEYTYPE key);
    void DB_nextrec();
};

#endif /* BTree_h */
