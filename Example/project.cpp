#include <iostream>
#include <fstream>
#include <string.h>
#include <sstream>
#include <cmath>
#include <list>
#include <unordered_map>
#include <algorithm>
#include <iomanip>

//#define _WYNNE_DEBUG_

using namespace std;

int address_bits , number_of_sets, associativity, block_size;
class Testcase{
public:
    char data[1000] = {0};
    int tag = 0;
    int index = 0;
    bool H_M  = 0;       // Miss = 1
};

class LRUCache{
private:
    // cache list: maintain new always at front
    list<int> cache;
    // able to find the position in cache list
    unordered_map<int, list<int>::iterator> cache_record;
    // new insert at front
    void insert_new_record(int tag){
        cache.push_front(tag);
        cache_record[tag] = cache.begin();
    }
    // erase
    void remove_record(int tag){
        cache.erase(cache_record[tag]);
    }
    // always pop from rear --> LRU
    void delete_one(){
        cache_record.erase(cache.back());
        cache.pop_back();
    }
public:
    int size;
    LRUCache(int n):size(n){}
    LRUCache(){}
    // find tag existing
    int find(int tag){
        if(cache_record.find(tag) == cache_record.end()){
            return -1;          // not found
        }
        // update cache to front
        remove_record(tag);
        insert_new_record(tag);
        return 0; // found
    }

    void put(int ntag){
        if( cache_record.size() == this->size){
            delete_one();
            insert_new_record(ntag);
        }else{
            insert_new_record(ntag);
        }
    }

};

int main(int argc, char *argv[]){
    // check command line
    if(argc != 4){
        cout << "wrong argument number." << endl;
        cout << "Example : ./arch_final_lsb cache.org reference.lst index.rpt" << endl;
        return 0;
    }

    //* read cache.org & store it to index.rpt
    ifstream fin;
    ofstream fout;
    fin.open( argv[1] , ios::in);
    if(fin.fail()){
    	cout << "Fail to open " << argv[1] << endl;
        return 0;
    }
    fout.open( argv[3] , ios::out);
    if(fout.fail()){
    	cout << "Fail to open " << argv[3] << endl;
        return 0;
    }
    string str;
    while(fin >> str){
        if(str == "Address_bits:"){
            fin >> address_bits;
        }else if(str == "Block_size:"){
            fin >> block_size;
        }else if(str == "Cache_sets:"){
            fin >> number_of_sets;
        }
        else if(str == "Associativity:"){
            fin >> associativity;
        }
        else {
            fout << "Input format is different from given file." << endl;
        }
    }
    fin.close();

    fout << "Address bits: " << address_bits << endl;
    fout << "Cache sets: " << number_of_sets << endl;
    fout << "Associativity: " << associativity << endl;
    fout << "Block size: " << block_size << endl;

    //* calculate index & offset cnt
    int index_cnt = 0, offset_cnt = 0;
    offset_cnt = log(block_size)/log(2);
    index_cnt = log(number_of_sets)/log(2);
    int index_bits[index_cnt] = {0};

    //* read reference.lst
    fin.open( argv[2] , ios::in);
    if(fin.fail()){
    	cout << "Fail to open " << argv[2] << endl;
        return 0;
    }
    string c;
    Testcase testcase[1000];
    int testcase_cnt = 0;
    while(getline(fin, c)){
        strcpy(testcase[testcase_cnt].data , c.data());
        testcase_cnt++;
        if( c == ".end") break;
    }
    fin.close();

    //__________________________________________________________________//
    //|                                                                |//
    //|                  How to choose index bits?                     |//
    //|________________________________________________________________|//
    //*----------------------------------------------------------------*//
    //*                  Q = min(z, o) / max(z, o)                     *//
    //*----------------------------------------------------------------*//
    int valid_bits = address_bits - offset_cnt;
    int testcases = testcase_cnt - 2; // first and last testcases are strings.
    double Z[valid_bits] = {0}, O[valid_bits] = {0}, Q[valid_bits] = {0};
    for(int i = 1; i <= testcases; i++){
        for(int j = 0, cnt = valid_bits - 1; j < valid_bits; j++, cnt--){
            if(testcase[i].data[j] == '0'){
                Z[cnt]++;  //a0 -> a[validbits-1]
            }
        }
    }
    for(int j = 0, cnt = valid_bits - 1; j < valid_bits; j++, cnt--){
        O[cnt] = testcases - Z[cnt];
    }

#ifdef _WYNNE_DEBUG_
    cout <<endl << "-------------------------------------------" << endl;
    cout << "testcase_cnt: " << testcase_cnt <<endl;
    cout << "testcases: " << testcases <<endl;
    cout << "valid_bits: " << valid_bits <<endl;
    cout <<endl << "-------------------------------------------" << endl;
#endif  // _WYNNE_DEBUG_
    for(int i = 0; i < valid_bits; i++){
        Q[i] = min(Z[i], O[i]) / max(Z[i], O[i]);
#ifdef _WYNNE_DEBUG_
        cout <<"Z[" << i <<"]: " << Z[i] <<"  ";
        cout <<"O[" << i <<"]: " << O[i] <<"  ";
        cout <<"Q[" << i <<"]: " << Q[i] <<"  ";
        cout << endl;
#endif  // _WYNNE_DEBUG_
    }

    //*----------------------------------------------------------------*//
    //*                  C = min(E, D) / max(E, D)                     *//
    //*----------------------------------------------------------------*//
    double E[valid_bits][valid_bits], D[valid_bits][valid_bits],
           C[valid_bits][valid_bits];
    // clear 2D array to zero
    for(int i = 0; i < valid_bits; i++){
        for(int j = 0; j < valid_bits; j++){
            E[i][j] = 0;
            C[i][j] = 0;
            D[i][j] = 0;
        }
    }
    // calculate Eij
    for(int i = 0, cnti = valid_bits - 1; i < valid_bits; i++, cnti--){
        for(int j = 0, cntj = valid_bits - 1; j < valid_bits; j++, cntj--){
            for(int k = 1; k <= testcases; k++){
                if(testcase[k].data[i] == testcase[k].data[j] ){
                    E[cnti][cntj]++;
                }
            }
        }
    }
    // calculate Dij
    for(int i = valid_bits - 1; i >= 0; i--){
        for(int j = valid_bits - 1; j >= 0; j--){
            D[i][j] = testcases - E[i][j];
        }
    }
    // calculate Cij
    for(int i = valid_bits - 1; i >= 0; i--){
        for(int j = valid_bits - 1; j >= 0; j--){
            D[i][j] = testcases - E[i][j];
            C[i][j] = min(E[i][j], D[i][j]) / max(E[i][j], D[i][j]);
        }
    }
#ifdef _WYNNE_DEBUG_
    cout <<endl << "-------------------------------------------" << endl;
    cout << "E ij" <<endl;
    for(int i = 0, cnti = valid_bits - 1; i < valid_bits; i++, cnti--){
        for(int j = 0, cntj = valid_bits - 1; j < valid_bits; j++, cntj--){
            cout << setw(10) << E[cnti][cntj];
        }
        cout << endl;
    }
    cout <<endl << "-------------------------------------------" << endl;
    cout << "D ij" <<endl;
    for(int i = 0, cnti = valid_bits - 1; i < valid_bits; i++, cnti--){
        for(int j = 0, cntj = valid_bits - 1; j < valid_bits; j++, cntj--){
            cout << setw(10) << D[cnti][cntj];
        }
        cout << endl;
    }
    cout <<endl << "-------------------------------------------" << endl;
    cout << "C ij" <<endl;
    for(int i = 0, cnti = valid_bits - 1; i < valid_bits; i++, cnti--){
        for(int j = 0, cntj = valid_bits - 1; j < valid_bits; j++, cntj--){
            cout << setw(10) << C[cnti][cntj];
        }
        cout << endl;
    }
    cout <<endl << "-------------------------------------------" << endl;
#endif  // _WYNNE_DEBUG_
    //*----------------------------------------------------------------*//
    //*             Compute Near-optimal Index Ordering                *//
    //*----------------------------------------------------------------*//
    int pick, pnt = 0;
    double temp = 0;
    bool dirty_pick[valid_bits] = {0};
    for(int i = 0; i < valid_bits; i++){
        pick = -1;
        temp = -1;
        //* find Qmax
        for(int k = 0; k < valid_bits; k++){
            if(dirty_pick[k] == 1){
                continue;
            }
            if(Q[k] >= temp){
                temp = Q[k];
                pick = k;
            }
        }
        index_bits[pnt] = pick;
        pnt++;
#ifdef _WYNNE_DEBUG_
        cout << "dirty pick: " << pick << endl;
#endif // _Wynne_DEBUG_
        for(int j = 0; j < valid_bits; j++){
            Q[j] = Q[j]*C[pick][j];
        }
        dirty_pick[pick] = 1;
    }

#ifdef _WYNNE_DEBUG_
    cout << endl << "------------------------------------------" << endl;
    cout <<"(without offset)Index bits: ";
    for(int i = 0; i < index_cnt; i++){
        cout << index_bits[i] << "  ";
    }
    cout << endl;
#endif  // _WYNNE_DEBUG_
    //__________________________________________________________________//
    //|                                                                |//
    //|                     Determine Hit / Miss                       |//
    //|________________________________________________________________|//
    //*----------------------------------------------------------------*//
    //*         Set all testcases' tag and index & Determine           *//
    //*----------------------------------------------------------------*//
    // Testcases are stored from data[0 ~ n-1].
    // Thus, reverse the bits we picked for determine H/M
    int start0_bit[valid_bits] = {0};
    int tag_bits[valid_bits] = {1};
    for(int i = 0; i < valid_bits; i++){
        start0_bit[i] = valid_bits - 1 - i;
    }
    for(int i = 0; i < valid_bits; i++){
        start0_bit[i] = start0_bit[index_bits[i]];
        tag_bits[i] = 1;
    }
    // for selecting the tag bits
    for(int i = 0; i < index_cnt; i++){
        tag_bits[start0_bit[i]] = 0;
    }
    int total_H_M = 0, idx;
    LRUCache LRUcache[number_of_sets];
    for(int i = 0; i < number_of_sets; i++){
        LRUcache[i] = *(new LRUCache(associativity));
    }
#ifdef _WYNNE_DEBUG_
    cout <<endl << "-------------------------------------------" << endl;
    cout << "tag: ";
    for(int i = 0; i < valid_bits; i++){
        cout << tag_bits[i] << " ";
    }
    cout <<endl << "-------------------------------------------" << endl;
#endif //_WYNNE_DEBUG_

    for(int i = 1; i <= testcases; i++){
        for(int j = 0; j < valid_bits; j++){ // allocate testcases' data(index tag)
            if(tag_bits[j]){
                if(testcase[i].data[j] == '1'){
                    testcase[i].tag = testcase[i].tag*2 + 1;
                }else{
                    testcase[i].tag *= 2;
                }
            }else{
                if(testcase[i].data[j] == '1'){
                    testcase[i].index = testcase[i].index*2 + 1;
                }else{
                    testcase[i].index *= 2;
                }
            }
        } // end of allocate testcases' data
#ifdef _WYNNE_DEBUG_
        cout << "testcase " << i << " index: " << testcase[i].index
             << " tag: " << testcase[i].tag << endl;
#endif //_WYNNE_DEBUG_

        // determine hit/miss
        idx = testcase[i].index % number_of_sets;
        if( LRUcache[idx].find(testcase[i].tag) == -1){ // miss
            LRUcache[idx].put(testcase[i].tag);
            testcase[i].H_M = 1;
            total_H_M++;
        }else{
            testcase[i].H_M = 0;
        }
    }

    //__________________________________________________________________//
    //|                                                                |//
    //|              store the last 2 part to index.rpt                |//
    //|________________________________________________________________|//
    fout << endl << "Offset bit count: " << offset_cnt << endl;
    fout << "Indexing bit count: " << index_cnt << endl;
    fout << "Indexing bits:" ;
    int actual_bits[index_cnt] ={0};
    for(int i = 0; i < index_cnt; i++){
        actual_bits[i] = index_bits[i] + offset_cnt;
    }
    sort(actual_bits, actual_bits + index_cnt, greater<int>());
    for(int i = 0; i < index_cnt; i++){
        fout << " " << actual_bits[i];
    }
    char strHM[2][5] = {"hit","miss"};
    fout << endl << endl;
    fout << testcase[0].data ;
    for(int i = 1; i < testcase_cnt-1; i++){
        fout << endl;
        fout << testcase[i].data << " " << strHM[testcase[i].H_M];
    }
    fout << endl << testcase[testcase_cnt-1].data << endl;
    fout << endl  << endl << "Total cache miss count: " << total_H_M << endl;

    fout.close();
    return 0;
}