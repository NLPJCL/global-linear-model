#pragma once
#include<iostream>
#include<unordered_map>
#include<vector>
#include<cmath>//自然对数的指数。
#include <algorithm>  
#include"dataset.h"
#include "time.h"
#include<numeric>
#include"windows.h"
#include<map>
using namespace std;
class global_linear_model
{
public:
	void create_feature_space();
	void online_training(bool averaged, bool shuffle, int iterator, int exitor);
	global_linear_model(string &train_, string &dev_, string &test_);
	//存储。
	void save_file(int i);
	~global_linear_model();
private:
	//基础数据集。
	dataset train;
	dataset dev;
	dataset test;
	unordered_map<string, int> model;//特征空间。
	map<string, int> tag;//词性
	vector<string> vector_tag;

	vector<int> w;
	vector<int> v;
	vector<int> update_time;

	vector<string> feature;
	vector<vector<int>> head_prob;
	//创建特征空间。
	vector<string> create_feature(const sentence &sentence, int pos);
	//在线算法
	void update_w(const sentence &sen,const vector<string> &max_sentence_tag ,int current_time);

	
	vector<string> max_score_sentence_tag(const sentence &sen,bool);
	
	vector<int> count_score(const vector<string> &feature,bool);
	
	vector<int> get_id(vector<string> &feature);
	
	//评价。
	
	double evaluate(dataset &, bool averaged);
};

#pragma once
