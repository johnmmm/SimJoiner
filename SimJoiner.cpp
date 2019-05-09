#include "SimJoiner.h"

using namespace std;

SimJoiner::SimJoiner() {
}

SimJoiner::~SimJoiner() {
}

int maxone (int a, int b)
{
    if (a > b)
        return a;
    return b;
}

unsigned long long SimJoiner::long_str_hash(string strs)
{
    unsigned long long hash_num = 0;
    int seed = 173;
    int len = strs.size();
    for (int i = 0; i < len; i++) {
        hash_num = hash_num * seed + (int)strs[i];
    }
    return hash_num & 0x7FFFFFFF;
}

unsigned SimJoiner::short_str_hash(string strs)
{
    unsigned long long hash_num = 0;
    int seed = 173;
    int len = strs.size();
    for (int i = 0; i < len; i++) {
        hash_num = hash_num * seed + (int)strs[i];
    }
    return hash_num & 0x3FFFFFF;
}

int SimJoiner::joinJaccard(const char *filename1, const char *filename2, double threshold, vector<JaccardJoinResult> &result) {
    result.clear();
    return SUCCESS;
}

int SimJoiner::joinED(const char *filename1, const char *filename2, unsigned threshold, vector<EDJoinResult> &result) {
    result.clear();

    create_index_ed(filename2, threshold);
    search_ed(filename1, threshold, result);
    //print_ed_result(result);

    return SUCCESS;
}

void SimJoiner::create_index_ed(const char *filename, unsigned threshold)
{
    unsigned place = 0;
    char buf[1024];
    FILE *fp = fopen(filename, "r");
	while (fgets(buf ,sizeof(buf), fp))
    {
        unsigned n = strlen(buf) - 1;
		buf[n] = '\0';
		string tmp_str = buf;

        unsigned long long str_hash = long_str_hash(tmp_str);
        strings_ed[place] = tmp_str;
        hash_num_ed[str_hash] = place;
        unsigned str_size = tmp_str.size();
        if (inverted_group_ed[str_size] == NULL)
        {
            vector<string>* new_group_ed;
            new_group_ed = new vector<string>;
            new_group_ed->push_back(tmp_str);
            inverted_group_ed[str_size] = new_group_ed;
        }
        else
            inverted_group_ed[str_size]->push_back(tmp_str);
        place++;
    }

    map<unsigned, vector<string>* >::iterator it;
    for (it = inverted_group_ed.begin(); it != inverted_group_ed.end(); it++)
    {
        if (inverted_list_ed[it->first] == NULL)
        {
            map<unsigned, map<unsigned long long, vector<unsigned> > >* new_inverted;
            new_inverted = new map<unsigned, map<unsigned long long, vector<unsigned> > >;
            inverted_list_ed[it->first] = new_inverted;
        }

        unsigned str_len = (it->second)->size();
        unsigned seg_len = it->first / (threshold + 1);
        unsigned long_seg_num = it->first % (threshold + 1);
        for (unsigned i = 0; i < str_len; i++)
        {
            unsigned sub_len = 0;
            unsigned sub_place = 0;
            for (unsigned j = 0; j < threshold + 1; j++)
            {
                if (threshold + 1 - j > long_seg_num)
                    sub_len = seg_len;
                else
                    sub_len = seg_len + 1;
                string new_tmp_str = (*(it->second))[i].substr(sub_place, sub_len);
                sub_place += sub_len;

                (*inverted_list_ed[it->first])[j][long_str_hash(new_tmp_str)].push_back(long_str_hash((*(it->second))[i]));
                //printf("%s ", new_tmp_str.c_str());
            }
            //printf("\n");
            //printf("%d: %s\n", it->first, (*(it->second))[i].c_str());
        }
        
    }
}

void SimJoiner::search_ed(const char *filename, unsigned threshold, vector<EDJoinResult> &result)
{
    unsigned place = 0;
    char buf[1024];
    FILE *fp = fopen(filename, "r");
	while (fgets(buf ,sizeof(buf), fp))
    {
        unsigned n = strlen(buf) - 1;
		buf[n] = '\0';
		string tmp_str = buf;

        //特殊处理一下短的串
        int tmp_str_len = tmp_str.size();

        int min_len = maxone(threshold+1, (tmp_str_len-(int)threshold));
        int max_len = tmp_str_len + threshold;

        set<unsigned> chosen_ones;

        for (int i = min_len; i <= max_len; i++)
        {
            if (inverted_list_ed[i] == NULL)
                continue;
            
            int seg_len = i / (threshold + 1);
            int long_seg_num = i % (threshold + 1);
            int sub_len = 0;
            int sub_place = 0;
            
            //这个改回string相比，hash没用
            for (int j = 0; j < threshold + 1; j++)
            {
                if (threshold + 1 - j > long_seg_num)
                    sub_len = seg_len;
                else
                    sub_len = seg_len + 1;

                int left_line = 0;
                int right_line = 0;
                int tmp_place = sub_place;
                while(tmp_place >= 0)
                {
                    if (tmp_place == 0)
                    {
                        left_line = tmp_place;
                        break;
                    }
                    tmp_place--;
                    if (abs(tmp_place - sub_place) + abs((tmp_str_len-sub_len-tmp_place) - (i-sub_len-sub_place)) > threshold)
                    {
                        left_line = tmp_place+1;
                        break;
                    }
                }

                tmp_place = sub_place;
                if (tmp_place+sub_len > tmp_str_len)
                    tmp_place = tmp_str_len - sub_len;
                while(tmp_place+sub_len <= tmp_str_len)
                {
                    if (tmp_place+sub_len == tmp_str_len)
                    {
                        right_line = tmp_place;
                        break;
                    }
                    tmp_place++;
                    if (abs(tmp_place - sub_place) + abs((tmp_str_len-sub_len-tmp_place) - (i-sub_len-sub_place)) > threshold)
                    {
                        right_line = tmp_place-1;
                        break;
                    }
                }
                //printf("%s: op:%d, left: %d, right: %d\n", tmp_str.c_str(), i, left_line, right_line);
                
                sub_place += sub_len;

                //计算其是否可行
                for (int k = left_line; k <= right_line; k++)
                {
                    string sub_strs = tmp_str.substr(k, sub_len);
                    int vec_size = (*inverted_list_ed[i])[j][long_str_hash(sub_strs)].size();
                    if (vec_size == 0)
                        continue;
                    else
                    {
                        for (int l = 0; l < vec_size; l++)
                        {
                            //printf("chosen: %d\n", hash_num_ed[(*inverted_list_ed[i])[j][long_str_hash(sub_strs)][l]]);
                            chosen_ones.insert(hash_num_ed[(*inverted_list_ed[i])[j][long_str_hash(sub_strs)][l]]);
                        }
                        continue;
                    }   
                }
            }
        }

        //计算chosen_one

        set<unsigned>::iterator it1;
        for (it1 = chosen_ones.begin(); it1 != chosen_ones.end(); it1++)
        {
            unsigned ed_dis = lenenshtein_distance(tmp_str, strings_ed[*it1], threshold);
            if (ed_dis <= threshold)
            {
                EDJoinResult new_result;
                new_result.id1 = place;
                new_result.id2 = *it1;
                result.push_back(new_result);
            }
        }
        place++;
    }
}

unsigned SimJoiner::lenenshtein_distance(string a, string b, unsigned threshold)
{
    int size_a = a.size();
    int size_b = b.size();
    int thres = (int)threshold;

    if (abs(size_a - size_b) > threshold)
        return MAXN;
    int dp[size_a+1][size_b+1];

    for (int i = 0; i <= min(thres, size_a); i++)
    {
        dp[i][0] = i;
    }
    for (int j = 0; j <= min(thres, size_b); j++)
    {
        dp[0][j] = j;
    }
    for (int i = 1; i <= size_a; i++)
    {
        int begin = max(i - thres, 1);
        int end = min(i + thres, size_b);
        if (begin > end)
            break;
        for (int j = begin; j <= end; j++)
        {
            int t = !(a[i - 1] == b[j - 1]);
            int d1 = abs(i - 1 - j) > threshold ? MAXN : dp[i - 1][j];
            int d2 = abs(i - j + 1) > threshold ? MAXN : dp[i][j - 1];
            dp[i][j] = min( min(d1+1, d2+1), dp[i-1][j-1]+t );            
        }
    }
    return (unsigned)dp[size_a][size_b];
}

void SimJoiner::print_ed_result(vector<EDJoinResult> &result)
{
    for (int i = 0; i < result.size(); i++)
    {
        printf("<%d, %d>\n", result[i].id1, result[i].id2);
    }
    
}