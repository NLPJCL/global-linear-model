﻿#include<iostream>
#include"dataset.h"
#include"global_linear_model.h"
using namespace std;
int main()
{
	bool averaged = true;
	bool shuffle = true;
	int iterator = 20;

	string train_data_file = "train.conll.txt";
	string dev_data_file = "dev.conll.txt";
	string test_data_file = "test.conll.txt";

	int exitor = 10;


	global_linear_model b(train_data_file, dev_data_file, test_data_file);
	b.create_feature_space();
	b.online_training(averaged, shuffle, iterator, exitor);
	system("pause");
	return 0;
}