#ifndef __EXP2_SIMJOINER_H__
#define __EXP2_SIMJOINER_H__

#include <vector>
#include <utility>
#include <cstdio>
#include <string>
#include <algorithm>
#include <cstring>
#include <map>
#include <unordered_map>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include <set>
#include <cstdlib>
using namespace std;

template <typename IDType, typename SimType>
struct JoinResult {
    IDType id1;
    IDType id2;
    SimType s;
};

typedef JoinResult<unsigned, double> JaccardJoinResult;
typedef JoinResult<unsigned, unsigned> EDJoinResult;

const int SUCCESS = 0;
const int FAILURE = 1;
const int MAXN = 6666666;
const double eps = 1e-8;
const int ed_hash = 2222222;
const int jac_hash = 76666666;
const int MAX_LEN = 256;
const int MAX_TAU = 3;

class SimJoiner {
public:
    SimJoiner();
    ~SimJoiner();

    void sort_idf(vector< vector<int> > *list, int *order);
    unsigned long long long_str_hash(string strs);
    unsigned short_str_hash(string strs);
    void insert_token(const string& str, vector<int> &res);
    void tokenize(string &str1, vector<int> &res);

    void clear_all_jac();
    void clear_all_ed();

    int joinJaccard(const char *filename1, const char *filename2, double threshold, std::vector<JaccardJoinResult> &result);
    int joinED(const char *filename1, const char *filename2, unsigned threshold, std::vector<EDJoinResult> &result);

    void create_index_ed(const char *filename, unsigned threshold);
    void search_ed(const char *filename, unsigned threshold, vector<EDJoinResult> &result);
    //上面这两个函数不能用，有bug。。。。
    double jaccard_distance(vector<int> &a, vector<int> &b);
    unsigned lenenshtein_distance(string a, string b, unsigned threshold);
    void print_ed_result(vector<EDJoinResult> &result);

    int layer = 0, tokens_num = 0, ivt_build = 1;
    vector< vector<int> > token1, token2;//idf表
    vector<int> tokens_used;//
    unordered_map<string, int> token_hash;//string和token值对应
    vector< vector< pair<int, int> > > inverted_list_jar;
    
    vector<unsigned> short_ones;
    map<unsigned, string> strings_ed;
    map<unsigned long long, unsigned> hash_num_ed;
    map<unsigned, vector<string>* > inverted_group_ed;
    map<unsigned, map<unsigned, map<unsigned long long, vector<unsigned long long> > >* > inverted_list_ed;

    //new way:
    void read_docs(const char *filename1, const char *filename2);
    
    void search_jac_new(double threshold, vector<JaccardJoinResult> &result);
    void search_ed_new(unsigned threshold, vector<EDJoinResult> &result);


};

#endif
