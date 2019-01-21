#include<iostream>
#include"dataset.h"
#include"global_linear_model.h"
using namespace std;
int main()
{
	global_linear_model b;
	b.create_feature_space();
	b.online_training();
	system("pause");
	return 0;
}