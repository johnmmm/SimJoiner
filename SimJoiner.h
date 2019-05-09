#ifndef __EXP2_SIMJOINER_H__
#define __EXP2_SIMJOINER_H__

#include <vector>
#include <utility>
#include <cstdio>
#include <string>
#include <algorithm>
#include <cstring>
#include <map>
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
const int MAXN = 66666666;

class SimJoiner {
public:
    SimJoiner();
    ~SimJoiner();

    unsigned long long long_str_hash(string strs);
    unsigned short_str_hash(string strs);

    int joinJaccard(const char *filename1, const char *filename2, double threshold, std::vector<JaccardJoinResult> &result);
    int joinED(const char *filename1, const char *filename2, unsigned threshold, std::vector<EDJoinResult> &result);

    void create_index_ed(const char *filename, unsigned threshold);
    void search_ed(const char *filename, unsigned threshold, vector<EDJoinResult> &result);
    unsigned lenenshtein_distance(string a, string b, unsigned threshold);
    void print_ed_result(vector<EDJoinResult> &result);

    map<unsigned, string> strings_ed;
    map<unsigned long long, unsigned> hash_num_ed;
    map<unsigned, vector<string>* > inverted_group_ed;
    map<unsigned, map<unsigned, map<unsigned long long, vector<unsigned> > >* > inverted_list_ed;


};

#endif
