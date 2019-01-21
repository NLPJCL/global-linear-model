#pragma once
#include<iostream>
#include<map>
#include<vector>
#include<cmath>//��Ȼ������ָ����
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
	//�洢��
	void save_file(int i);
	~global_linear_model();
private:
	//�������ݼ���
	dataset train;
	dataset dev;
	dataset test;
	map<string, int> model;//�����ռ䡣
	map<string, int> tag;//����
	vector<int> w;//һά������Ȩ�ء�
	vector<string> vector_tag;
	vector<string> feature;//
	vector<vector<int>> head_prob;
	//���������ռ䡣
	vector<string> create_feature(const sentence &sentence, int pos);
	//�����㷨
	void update_w(const sentence &sen,const vector<string> &max_sentence_tag );
	vector<string> max_score_sentence_tag(const sentence &sen);
	vector<int> count_score(const vector<string> &feature);
	vector<int> get_id(vector<string> &feature);
	//���ۡ�
	double evaluate(dataset);
};

#pragma once
