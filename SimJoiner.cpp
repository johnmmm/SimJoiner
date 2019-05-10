#include "SimJoiner.h"

using namespace std;

// vector<unsigned>* inverted_list_ed_hash[ed_hash];
// unsigned jac_is_dup[jac_hash];

int *token_place = new int[MAXN];
int *token_order = new int[MAXN];  //token的顺序
int *count_filter = new int[MAXN];
pair<int, int> *token_nums = new pair<int, int>[MAXN];//每个token出现从次数
vector<string> strs1, strs2;
map<unsigned long long, vector<int> > new_inverted_list_ed[MAX_LEN+2][MAX_TAU+2];


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

int minone (int a, int b)
{
    if (a < b)
        return a;
    return b;
}

bool cmp_result_ed(EDJoinResult a, EDJoinResult b) //升序排序
{
    if (a.id1 == b.id1)
        return a.id2 < b.id2;
    return a.id1 < b.id1;
}

bool cmp_idf_count(pair<int, int> a, pair<int, int> b) //按idf，升序排序
{ 
    return a.second < b.second;
}

void SimJoiner::sort_idf(vector< vector<int> > *list, int *order){  //ok
    for (auto &i : *list){
        for (auto &j : i)
            j = order[j];
        sort(i.begin(), i.end());
    }
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

void SimJoiner::insert_token(const string& str, vector<int> &res)
{
    //printf("insert?\n");
    if(!token_hash.count(str))
    {  
        //printf("thisone?\n");
        token_hash[str] = tokens_num;
        //printf("which?\n");
        //token_hash.insert({str, tokens_num});
        token_nums[tokens_num].second = 1;
        //printf("which?\n");
        res.emplace_back(tokens_num);
        //printf("which?\n");
        token_place[tokens_num] = layer;
        tokens_num++;
        //printf("which?\n");
    }
    else
    {  
        //printf("thisone??\n");
        auto it = token_hash.find(str);
        if(token_place[it->second] != layer)
        {  
            token_nums[it->second].second++;
            token_place[it->second] = layer;
            res.emplace_back(it->second);
        }
    }
}

void SimJoiner::tokenize(string &str1, vector<int> &res)
{
    res.clear();
	string::size_type p1, p2;
	p1 = 0;  p2 = str1.find(" ");
    //printf("tokenize?\n");

	while(string::npos != p2)
    {
		//printf("twice?\n");
        if(p1 != p2)
        {
            string tmp = str1.substr(p1, p2-p1);
            insert_token(tmp, res);
		}
		p1 = p2 + 1;
		p2 = str1.find(" ", p1);
	}
	if(p1 != str1.length())
    {
        string tmp = str1.substr(p1);
        insert_token(tmp, res);
	}
}

void SimJoiner::clear_all_jac()
{
    token1.clear();
    token2.clear();
    tokens_used.clear();
    token_hash.clear();

    memset(token_place, MAXN, sizeof(int));
    memset(token_order, MAXN, sizeof(int));
    memset(count_filter, MAXN, sizeof(int));
    for(int i = 0; i < MAXN; i++)
    {
        token_nums[i] = {i, 0};
        tokens_used.emplace_back(0);
        inverted_list_jar.emplace_back();
    }
}

void SimJoiner::clear_all_ed()
{
    strings_ed.clear();
    hash_num_ed.clear();
    inverted_group_ed.clear();
    inverted_list_ed.clear();
    strs1.clear();
    strs2.clear();

    for (int i = 0; i < MAX_LEN+1; i++)
    {
        for (int j = 0; j < MAX_TAU+1; j++)
        {
            new_inverted_list_ed[i][j].clear();
        }
    }
}

int SimJoiner::joinJaccard(const char *filename1, const char *filename2, double threshold, vector<JaccardJoinResult> &result) {
    result.clear();
    //result.emplace_back((JaccardJoinResult){0, 0, 0.99});

    clear_all_jac();
    read_docs(filename1, filename2);
    search_jac_new(threshold, result);

    return SUCCESS;
}

int SimJoiner::joinED(const char *filename1, const char *filename2, unsigned threshold, vector<EDJoinResult> &result) {
    result.clear();

    clear_all_ed();
    // create_index_ed(filename2, threshold);
    // search_ed(filename1, threshold, result);
    // sort(result.begin(), result.end(), cmp_result_ed);
    // print_ed_result(result);

    read_docs(filename1, filename2);
    search_ed_new(threshold, result);
    //print_ed_result(result);

    return SUCCESS;
}

void SimJoiner::create_index_ed(const char *filename, unsigned threshold)
{
    unsigned place = 0;
    char buf[1024];
    FILE *fp = fopen(filename, "r");
	while (fgets(buf ,sizeof(buf), fp) != nullptr)
    {
        string tmp_str = string(buf);
		if(tmp_str[tmp_str.length()-1] == '\n')
			tmp_str.erase(tmp_str.length()-1);
        
        unsigned long long str_hash = long_str_hash(tmp_str);
        strings_ed[place] = tmp_str;
        hash_num_ed[str_hash] = place;
        unsigned str_size = tmp_str.size();

        if (inverted_group_ed[str_size] == NULL)
        {
            vector<string>* new_group_ed;
            new_group_ed = new vector<string>;
            new_group_ed->clear();
            new_group_ed->emplace_back(tmp_str);
            inverted_group_ed[str_size] = new_group_ed;
        }
        else
            inverted_group_ed[str_size]->emplace_back(tmp_str);
        place++;
    }

    map<unsigned, vector<string>* >::iterator it;
    for (it = inverted_group_ed.begin(); it != inverted_group_ed.end(); it++)
    {
        if (inverted_list_ed[it->first] == NULL)
        {
            map<unsigned, map<unsigned long long, vector<unsigned long long> > >* new_inverted;
            new_inverted = new map<unsigned, map<unsigned long long, vector<unsigned long long> > >;
            new_inverted->clear();
            inverted_list_ed[it->first] = new_inverted;
        }

        unsigned str_nums = (it->second)->size();
        unsigned seg_len = it->first / (threshold + 1);
        unsigned long_seg_num = it->first % (threshold + 1);
        for (unsigned i = 0; i < str_nums; i++)
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

                (*inverted_list_ed[it->first])[j][long_str_hash(new_tmp_str)].emplace_back(long_str_hash((*(it->second))[i]));
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
	while (fgets(buf ,sizeof(buf), fp) != nullptr)
    {
        string tmp_str = string(buf);
		if(tmp_str[tmp_str.length()-1] == '\n')
			tmp_str.erase(tmp_str.length()-1);

        //特殊处理一下短的串
        int tmp_str_len = tmp_str.size();

        int min_len = maxone(0, (tmp_str_len-(int)threshold));
        int max_len = tmp_str_len + (int)threshold;

        set<unsigned> chosen_ones;
        chosen_ones.clear();

        for (int i = min_len; i <= max_len; i++)
        {
            if (inverted_list_ed[i] == NULL)
                continue;
            
            int seg_len = i / ((int)threshold + 1);
            int long_seg_num = i % ((int)threshold + 1);
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
                
                left_line = maxone(0, sub_place - threshold);
                right_line = minone(tmp_str_len - sub_len, sub_place + threshold);
                left_line = 0;
                right_line = tmp_str_len - sub_len;
                sub_place += sub_len;


                //计算其是否可行
                for (int k = left_line; k <= right_line; k++)
                {
                    string sub_strs = tmp_str.substr(k, sub_len);
                    
                    if ((*inverted_list_ed[i])[j].count(long_str_hash(sub_strs)) == 0)
                        continue;
                    else
                    {
                        int vec_size = (*inverted_list_ed[i])[j][long_str_hash(sub_strs)].size();
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
                new_result.s = ed_dis;
                result.emplace_back(new_result);
            }
        }

        chosen_ones.clear();
        place++;
    }
}

double SimJoiner::jaccard_distance(vector<int> &a, vector<int> &b)
{
    //排好序的vector
    int i = 0, j = 0;
    double overlap = 0;
    while(i < a.size() && j < b.size())
    {
        if(a[i] == b[j])
        {
            i++; j++;
            overlap++;
        }
        else if(a[i] < b[j])
            i++;
        else    
            j++;
    }
    return (double)(overlap / (a.size() + b.size() - overlap));
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
        printf("<%d, %d, %d>\n", result[i].id1, result[i].id2, result[i].s);
    }
    
}

void SimJoiner::read_docs(const char *filename1, const char *filename2)
{
    char buf[1024];
    strs1.clear();
    strs2.clear();
    //先读第一个
    FILE *fp1 = fopen(filename1, "r");
	while (fgets(buf ,sizeof(buf), fp1) != nullptr)
    {
        string tmp_str = string(buf);
		if(tmp_str[tmp_str.length()-1] == '\n')
			tmp_str.erase(tmp_str.length()-1);
        strs1.emplace_back(tmp_str);
    }
    fclose(fp1);

    //读第二个
    FILE *fp2 = fopen(filename2, "r");
	while (fgets(buf ,sizeof(buf), fp2) != nullptr)
    {
        string tmp_str = string(buf);
		if(tmp_str[tmp_str.length()-1] == '\n')
			tmp_str.erase(tmp_str.length()-1);
        strs2.emplace_back(tmp_str);
    }
    fclose(fp2);
}

void SimJoiner::search_jac_new(double threshold, vector<JaccardJoinResult> &result)
{
    //int tau_1 = (int)(threshold + 1);

    //printf("read?\n");
    //idf建立
    vector<int> token;
    for (int i = 0; i < strs1.size(); i++)
    {  
        token.clear();
        layer++;
        tokenize(strs1[i], token);
        token1.emplace_back(token);
    } 
    for (int i = 0; i < strs2.size(); i++)
    {
        token.clear();
        layer++;
        tokenize(strs2[i], token);
        token2.emplace_back(token);
    }

    // for (int i = 0; i < token1.size(); i++)
    // {
    //     for (int j = 0; j < token1[i].size(); j++)
    //     {
    //         printf("%d ", token1[i][j]);
    //     }
    //     printf("\n");
    // }

    // printf("nums: %d\n", tokens_num);
    sort(token_nums, token_nums+tokens_num, cmp_idf_count);  //get idf
    // for(int i = 0; i < tokens_num; i++)  
    //     printf("token_nums:(%d, %d)\n", token_nums[i].first, token_nums[i].second);
    //idf大的放后面，默认相同的后面
    for(int i = 0; i < tokens_num; i++)  
        token_order[token_nums[i].first] = i;

    //排序
    sort_idf(&token1, token_order);
    sort_idf(&token2, token_order);

    // printf("after sort!!!!\n");

    // for (int i = 0; i < token1.size(); i++)
    // {
    //     for (int j = 0; j < token1[i].size(); j++)
    //     {
    //         printf("%d ", token1[i][j]);
    //     }
    //     printf("\n");
    // }

    //建立前缀倒排列表
    //printf("here?\n");
    int pre_len = 0;
    double thata = 1-threshold;
    for(int i = 0; i < token2.size(); i++)
    {
        pre_len = (int)floor(thata * token2[i].size() + 1);
        for(int j = 0; j < pre_len; j++)
        {
            if(tokens_used[token2[i][j]] != ivt_build)
            {  
                inverted_list_jar[token2[i][j]].clear();
                tokens_used[token2[i][j]] = ivt_build;
            }
            if(inverted_list_jar[token2[i][j]].empty() || inverted_list_jar[token2[i][j]].back().first != i)
                inverted_list_jar[token2[i][j]].emplace_back(i, j);
        }
    }

    // //step2 根据共同前缀，挑选候选项
    // //printf("here?\n");
    double low_th = threshold / (1 + threshold);
    for(unsigned i = 0; i < token1.size(); i++)
    {
        pre_len = (int)floor(thata * token1[i].size() + 1);
        vector<int> chosen_ones;
        layer++;
        for(int j = 0; j < pre_len; j++)
        {
            if(tokens_used[token1[i][j]] == ivt_build)
            {
                for(auto &it : inverted_list_jar[token1[i][j]])
                {
                    if(token_place[it.first] != layer)
                    {  
                        chosen_ones.emplace_back(it.first);
                        token_place[it.first] = layer;
                        count_filter[it.first] = 0;
                    }
                    int x = token1[i].size(), y = token2[it.first].size();
                    int low_bound = (int)ceil(low_th * (int)(x + y));  
                    int cur_low_bound = 1 + min((int)x-j, (int)y-it.second);
                    if(cur_low_bound >= low_bound)
                        count_filter[it.first] = 1;
                } 
            }
        }

        sort(chosen_ones.begin(), chosen_ones.end());
        for(auto &it : chosen_ones)
        { //验证
            if(count_filter[it] <= 0) 
                continue;
            double t = jaccard_distance(token1[i], token2[it]);
            if(t + eps >= threshold)
                result.emplace_back((JaccardJoinResult){i, (unsigned)it, t});
        }
    }
}

void SimJoiner::search_ed_new(unsigned threshold, vector<EDJoinResult> &result)
{
    int tau_1 = (int)(threshold + 1);

    for(int i = 0; i < strs2.size(); i++)
    {
        int len = (int)strs2[i].size();
        int seg_len = len / tau_1;
        int long_len = 1 + seg_len;
        int long_seg_num = len % tau_1;

        int p = 0; 
        int cur_len, part_idx = 0;
        while(part_idx < tau_1)
        {
            //选择切分长短链
            cur_len = seg_len;
            if(long_seg_num > 0){
                cur_len = long_len;
                long_seg_num--;
            }
            string tmp = strs2[i].substr((unsigned)p, (unsigned)cur_len);
            unsigned long long tmp_hash = long_str_hash(tmp);

            //存储同一切分位置，不同的字符串出现idx
            if(new_inverted_list_ed[len][part_idx].count(tmp_hash) == 0){
                new_inverted_list_ed[len][part_idx][tmp_hash] = vector<int>();
            }
            new_inverted_list_ed[len][part_idx][tmp_hash].emplace_back(i);

            p += cur_len;
            part_idx++;
        }
    }

    vector<int> chosen_ones;
    for(unsigned i = 0; i < strs1.size(); i++)
    {
        chosen_ones.clear();
        int len = (int)strs1[i].size();

        int min_len = maxone(0, len - (int)threshold);
        int max_len = len + (int)threshold;
        for (unsigned j = min_len; j <= max_len; j++)
        {
            if (new_inverted_list_ed[j][0].size() == 0)
                continue;

            int seg_len = j / tau_1;
            int long_len = 1 + seg_len;
            int long_seg_num = j % tau_1;

            int p = 0; 
            int cur_len, part_idx = 0;
            while(part_idx < tau_1)
            {
                cur_len = seg_len;
                if(long_seg_num > 0)
                {
                    cur_len = long_len;
                    long_seg_num--;
                }
                string tmp;
                unsigned long long tmp_hash;
                int left_line = 0;
                int right_line = len-cur_len;
                int delta = abs(len-(int)j);
                int tau = (int)threshold;
                int undetected_seg = tau-part_idx;
                // int small_left = p - floor((tau - delta) / 2);
                // int big_right = p + floor((tau - delta) / 2);
                left_line = maxone(0, p-(int)threshold);
                right_line = minone(p+(int)threshold, len-cur_len);
                // left_line = maxone(0, small_left);
                // right_line = minone(big_right, len-cur_len);
                if (j <= len)
                {
                    left_line = maxone(p-part_idx, p+delta-undetected_seg);
                    right_line = minone(p+part_idx, p+delta+undetected_seg);
                }

                for (unsigned k = left_line; k <= right_line; k++)
                {
                    tmp = strs1[i].substr(k, cur_len);
                    tmp_hash = long_str_hash(tmp);
                    if(new_inverted_list_ed[j][part_idx].count(tmp_hash) != 0)
                        chosen_ones.insert(chosen_ones.end(), new_inverted_list_ed[j][part_idx][tmp_hash].begin(), new_inverted_list_ed[j][part_idx][tmp_hash].end());
                }
                p += cur_len;
                part_idx++;
            }
        }

        sort(chosen_ones.begin(), chosen_ones.end());
        chosen_ones.erase(unique(chosen_ones.begin(), chosen_ones.end()), chosen_ones.end());

        for(auto &it : chosen_ones)
        {  
            auto t = lenenshtein_distance(strs1[i], strs2[it], threshold);
            if(t <= threshold)
                result.emplace_back((EDJoinResult){i, (unsigned)it, t});
        }
    }
}