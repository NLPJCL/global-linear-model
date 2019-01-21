#pragma once
#include<iostream>
#include<map>
#include<vector>
#include<cmath>//自然对数的指数。
#include <algorithm>  
#include"dataset.h"
#include "time.h"
#include<numeric>
#include"windows.h"
using namespace std;
class global_linear_model
{
public:
	void create_feature_space();
	void online_training();
	global_linear_model();
	//存储。
	void save_file(int i);
	~global_linear_model();
private:
	//基础数据集。
	dataset train;
	dataset dev;
	dataset test;
	map<string, int> model;//特征空间。
	map<string, int> tag;//词性
	vector<int> w;//一维的特征权重。
	vector<string> vector_tag;
	vector<string> feature;//
	vector<vector<int>> head_prob;
	//创建特征空间。
	vector<string> create_feature(const sentence &sentence, int pos);
	//在线算法
	void update_w(const sentence &sen,const vector<string> &max_sentence_tag );
	vector<string> max_score_sentence_tag(const sentence &sen);
	vector<int> count_score(const vector<string> &feature);
	vector<int> get_id(vector<string> &feature);
	//评价。
	double evaluate(dataset);
};

#pragma once
