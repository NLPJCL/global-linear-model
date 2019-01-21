#include "global_linear_model.h"
//实例化特征
vector<string> global_linear_model::create_feature(const sentence &sentence, int pos)
{
	string word = sentence.word[pos];//当前词。
	string word_char_first = sentence.word_char[pos][0];//当前词的第一个元素。
	string word_char_last = sentence.word_char[pos][sentence.word_char[pos].size() - 1];//当前词的最后一个元素。
	string word_m1;
	string word_char_m1m1;
	string word_p1;
	string word_char_p1_first;
	int word_count = sentence.word.size();//当前句的总词数。
	if (pos == 0)
	{
		word_m1 = "$$";
		word_char_m1m1 = "$";
	}
	else
	{
		word_m1 = sentence.word[pos - 1];
		word_char_m1m1 = sentence.word_char[pos - 1][(sentence.word_char[pos - 1].size() - 1)];
	}
	if (pos == word_count - 1)
	{
		word_p1 = "##";
		word_char_p1_first = "#";
	}
	else
	{
		word_p1 = sentence.word[pos + 1];
		word_char_p1_first = sentence.word_char[pos + 1][0];
	}
	vector<string> f;
	f.reserve(50);
	f.emplace_back("02:*" + word);
	f.emplace_back("03:*" + word_m1);
	f.emplace_back("04:*" + word_p1);
	f.emplace_back("05:*" + word + "*" + word_char_m1m1);
	f.emplace_back("06:*" + word + "*" + word_char_p1_first);
	f.emplace_back("07:*" + word_char_first);
	f.emplace_back("08:*" + word_char_last);
	int pos_word_len = sentence.word_char[pos].size();
	for (int k = 1; k < pos_word_len - 1; k++)
	{
		string cik = sentence.word_char[pos][k];
		f.emplace_back("09:*" + cik);
		f.emplace_back("10:*" + word_char_first + "*" + cik);
		f.emplace_back("11:*" + word_char_last + "*" + cik);
		string cikp1 = sentence.word_char[pos][k + 1];
		if (cik == cikp1)
		{
			f.emplace_back("13:*" + cik + "*" + "consecutive");
		}
	}
	if (sentence.word_char[pos].size() > 1)
	{
		if (sentence.word_char[pos][0] == sentence.word_char[pos][1])
			f.emplace_back("13:*" + sentence.word_char[pos][0] + "*" + "consecutive");
	}
	if (pos_word_len == 1)
	{
		f.emplace_back("12:*" + word + "*" + word_char_m1m1 + "*" + word_char_p1_first);
	}
	for (int k = 0; k <pos_word_len; k++)
	{
		if (k >= 4)break;
		f.emplace_back("14:*" + accumulate(sentence.word_char[pos].begin(), sentence.word_char[pos].begin() + k + 1, string("")));
		f.emplace_back("15:*" + accumulate(sentence.word_char[pos].end() - (k + 1), sentence.word_char[pos].end(), string("")));
	}
	return f;
}
void global_linear_model::create_feature_space()
{
	int word_count = 0, tag_count = 0;
	for (auto sen = train.sentences.begin(); sen != train.sentences.end(); ++sen)
	{
		for (int i = 0; i < sen->word.size(); i++)
		{
			vector<string> f = create_feature(*sen, i);

			if (i == 0)
			{
				f.emplace_back("01:*&&");
			}
			else
			{
				f.emplace_back("01:*" + sen->tag[i - 1]);
			}

			for (auto i = f.begin(); i != f.end(); ++i)
			{
				if (model.find(*i) == model.end())
				{
					model[*i] = word_count;
					feature.emplace_back(*i);
					word_count++;
				}
			}
			if (tag.find(sen->tag[i]) == tag.end())
			{
				tag[sen->tag[i]] = tag_count;
				vector_tag.emplace_back(sen->tag[i]);
				tag_count++;
			}
		}
	}
	w.reserve(tag.size()*model.size());
	v.reserve(tag.size()*model.size());
	update_time.reserve(tag.size()*model.size());
	for (int j = 0; j < tag.size()*model.size(); j++)
	{
		w.emplace_back(0);
		v.emplace_back(0);
		update_time.emplace_back(0);
	}

	vector<vector<int>> c(tag.size(), vector<int>(tag.size(), 0));
	head_prob = c;
	cout << "the total number of features is " << model.size() << endl;
	cout << "the total number of tags is " << tag.size() << endl;
}
vector<int> global_linear_model::get_id(vector<string> &feature)
{
  	vector<int> feature_id;
  	for (int i = 0; i < feature.size(); i++)
  	{
  		if (model.find(feature[i]) != model.end())
  		{
	  			feature_id.emplace_back(model[feature[i]]);
  		}
  	}
  	return feature_id;
}

vector<int> global_linear_model::count_score(const vector<string> &feature, bool averaged)
{
	int offset = 0;
	vector<vector<int>> scores(tag.size(), vector<int>(feature.size(), 0));
	if (averaged)
	{
		for (int j = 0; j < tag.size(); j++)
		{
			offset = j * model.size();
			for (int i = 0; i < feature.size(); i++)
			{
				if (model.find(feature[i]) != model.end())
				{
					scores[j][i] = v[model[feature[i]] + offset];
				}
			}
		}
	}
	else
	{
		for (int j = 0; j < tag.size(); j++)
		{
			offset = j * model.size();
			for (int i = 0; i < feature.size(); i++)
			{
				if (model.find(feature[i]) != model.end())
				{
					scores[j][i] = w[model[feature[i]] + offset];
				}
			}
		}
	}
	vector<int> score(tag.size());
	for (int j=0;j<tag.size();j++)
	{
		score[j] = accumulate(scores[j].begin(),scores[j].end(),0);
	}
	return score;
}
void global_linear_model::update_w(const sentence &sen, const vector<string> &max_sentence_tag,int current_time)
{
		int last_w_value;
		int last_time;
		//第一个词
		vector<string> feature=create_feature(sen, 0);
		feature.emplace_back("01:*&&");
		vector<int> fv_id=get_id(feature);
		int correct_offset = tag[sen.tag[0]] * model.size();
		int max_offset = tag[max_sentence_tag[0]] * model.size();
		for (int j = 0; j < fv_id.size(); j++)
		{
			//更新正确的权重
			int correct_index = correct_offset + fv_id[j];
			last_w_value = w[correct_index];
			w[correct_index]++;
			//更新v
			last_time = update_time[correct_index];
			update_time[correct_index] = current_time;
			v[correct_index] += (current_time - last_time - 1)*last_w_value + w[correct_index]; //更新权值。


			//更新错误的权重。
			int max_index = max_offset + fv_id[j];
			last_w_value = w[max_index];
			w[max_index]--;
			//更新v
			last_time = update_time[max_index];
			update_time[max_index] = current_time;
			v[max_index] += (current_time - last_time - 1)*last_w_value + w[max_index]; //更新权值。
		}
		//其余的词。
		for (int i = 1; i < sen.word.size(); i++)
		{
			//实际构造的特征只有第一个不一样（即前面的词性），其他的都一样，所以只构造一个。//分开加减即可。
			
			//处理第一部分的特征，
			//正确的加。
			int correct_offset = tag[sen.tag[i]] * model.size();
			int correct_index = correct_offset + model["01:*" + sen.tag[i - 1]];
			last_w_value = w[correct_index];
			w[correct_index]++;
			//更新v
			last_time = update_time[correct_index];
			update_time[correct_index] = current_time;
			v[correct_index] += (current_time - last_time - 1)*last_w_value + w[correct_index]; //更新权值。

																								//错误的减。
			int max_offset = tag[max_sentence_tag[i]] * model.size();
			int max_index = max_offset + model["01:*" + max_sentence_tag[i - 1]];
			last_w_value = w[max_index];
			w[max_index]--;
			//更新v
			last_time = update_time[max_index];
			update_time[max_index] = current_time;
			v[max_index] += (current_time - last_time - 1)*last_w_value + w[max_index]; //更新权值。

			//处理其余特征。
			vector<string> correct_feature = create_feature(sen, i);
			vector<int> correct_fv_id=get_id(correct_feature);
			//从02开始的特征都是一样的，不管正确的还是错误的。
			for (int j = 0; j < correct_fv_id.size(); j++)
			{
				//更新正确
				int correct_index = correct_offset + correct_fv_id[j];
				last_w_value = w[correct_index];
				w[correct_index]++;
				//更新v
				last_time = update_time[correct_index];
				update_time[correct_index] = current_time;
				v[correct_index] += (current_time - last_time - 1)*last_w_value + w[correct_index]; //更新权值。
				//更新错误
				int max_index = max_offset + correct_fv_id[j];
				last_w_value = w[max_index];
				w[max_index]--;
				//更新v
				last_time = update_time[max_index];
				update_time[max_index] = current_time;
				v[max_index] += (current_time - last_time - 1)*last_w_value + w[max_index]; //更新权值。
			}
		}
}
vector<string> global_linear_model::max_score_sentence_tag(const sentence &sen,bool averaged)
{
	vector<vector<int>> score_prob(sen.tag.size(), vector<int>(tag.size(), 0));
	vector<vector<int>> score_path(sen.tag.size(), vector<int>(tag.size(), 0));
	for (int i = 0; i < tag.size(); i++)
	{
		score_path[0][i] = -1;
	}
	//第一个词。
	vector <string> feature=create_feature(sen, 0);
	feature.emplace_back("01:*&&");
	score_prob[0] = count_score(feature,averaged);
	//其余的词。
	for (int j = 1; j < sen.word.size(); j++)
	{

			vector<int> currect_prob(tag.size(), 0);
			vector <string> feature=create_feature(sen, j);	
			vector<int> score = count_score(feature,averaged);
			for (int i = 0; i < tag.size(); i++)
			{
				for (int z = 0; z < tag.size(); z++)
				{
					currect_prob[z] = head_prob[i][z] + score_prob[j - 1][z];
				}
				auto w = max_element(currect_prob.begin(), currect_prob.end());
				score_prob[j][i] = *w+score[i];
				score_path[j][i] = distance(currect_prob.begin(), w);
			}
	}
	vector<string> path_max;
	vector<int> a = score_prob[sen.word.size() - 1];
	auto w = max_element(begin(a), end(a));
	int max_point = distance(begin(a),w);
	for (int j = sen.word.size() - 1; j > 0; j--)
	{
		path_max.emplace_back(vector_tag[max_point]);
		max_point = score_path[j][max_point];
	}
	path_max.emplace_back(vector_tag[max_point]);
	reverse(path_max.begin(), path_max.end());
	return path_max;
}
double global_linear_model::evaluate(dataset &data, bool averaged)
{
	int correct = 0, all = 0;
	for (auto sen = data.sentences.begin(); sen!= data.sentences.end(); sen++)
	{
		vector<string> max_sentence_tag=max_score_sentence_tag(*sen,averaged);
		for (int i = 0; i < max_sentence_tag.size(); i++)
		{
			if (max_sentence_tag[i] == sen->tag[i])
			{
				correct++;
			}
			all++;
		}
	}
	return (correct / double(all));
}
void global_linear_model::online_training(bool averaged, bool shuffle, int iterator, int exitor)
{

	//构建文件名。
	string file_name;
	if (test.name.size() != 0)
	{
		file_name = "big_result_";
	}
	else
	{
		file_name = "small_result_";
	}
	if (shuffle)
	{
		file_name += "shuffle_";
	}
	if (averaged)
	{
		file_name += "averaged";
	}
	file_name += ".txt";

	ofstream result(file_name);
	if (!result)
	{
		cout << "don't open  file" << endl;
	}
	result << train.name << "共" << train.sentence_count << "个句子，共" << train.word_count << "个词" << endl;
	result << dev.name << "共" << dev.sentence_count << "个句子，共" << dev.word_count << "个词" << endl;
	if (test.name.size() != 0)
	{
		result << test.name << "共" << test.sentence_count << "个句子，共" << test.word_count << "个词" << endl;
	}
	result << " the total number of features is " << model.size() << endl;
	int update_times = 0;
	int count = 0;
	double max_train_precision = 0;
	int max_train_iterator = 0;

	double max_dev_precision = 0;
	int max_dev_iterator = 0;

	double max_test_precision = 0;
	int max_test_iterator = 0;

	if (averaged)
	{
		cout << "using V to predict dev data..." << endl;
		result << "using V to predict dev data..." << endl;
	}
	else
	{
		cout << "using W to predict dev data..." << endl;
		result << "using w to predict dev data..." << endl;
	}
	DWORD t1, t2, t3, t4;
	t1 = timeGetTime();
	for (int j = 0; j < iterator; j++)
	{
		cout << "iterator" << j << endl;
		result << "iterator " << j << endl;
		t3 = timeGetTime();
		if (shuffle)
		{
			cout << "正在打乱数据" << endl;
			train.shuffle();
		}
		for (auto sen = train.sentences.begin(); sen != train.sentences.end(); ++sen)
		{
				vector<string> max_sentence_tag=max_score_sentence_tag(*sen, false);
				if (max_sentence_tag != sen->tag)
				{
					update_times++;
					update_w(*sen, max_sentence_tag,update_times);
				}
				for (int i = 0; i < tag.size(); i++)
				{
					int index = i * model.size();
					for (int j = 0; j < tag.size(); j++)
					{
							int max_tag_id = model["01:*" + vector_tag[j]];
							head_prob[i][j] = w[index + max_tag_id];
					}
				}
		}
		//save_file(j);
		if (averaged)
		{
			int last_time, last_w_value;
			int current_time = update_times;
			for (int i = 0; i < v.size(); i++)
			{
				last_time = update_time[i];
				last_w_value = w[i];
				if (last_time != current_time)
				{
					update_time[i] = current_time;
					v[i] += (current_time - last_time - 1)*last_w_value; //更新权值。
				}
			}
			for (int i = 0; i < tag.size(); i++)
			{
				int index = i * model.size();
				for (int j = 0; j < tag.size(); j++)
				{
					int max_tag_id = model["01:*" + vector_tag[j]];
					head_prob[i][j] = v[index + max_tag_id];
				}
			}
		}
		double train_precision = evaluate(train, averaged);
		cout << train.name << "=" << train_precision << endl;
		result << train.name << "=" << train_precision << endl;

		double dev_precision = evaluate(dev, averaged);
		cout << dev.name << "=" << dev_precision << endl;
		result << dev.name << "=" << dev_precision << endl;

		if (train_precision > max_train_precision)
		{
			max_train_precision = train_precision;
			max_train_iterator = j;
		}
		if (dev_precision > max_dev_precision)
		{
			max_dev_precision = dev_precision;
			max_dev_iterator = j;
			count = 0;
		}
		else
		{
			count++;
		}
		if (test.name.size() != 0)
		{
			double test_precision = evaluate(test, averaged);
			cout << test.name << "=" << test_precision << endl;
			result << test.name << "=" << test_precision << endl;
			if (test_precision > max_test_precision)
			{
				max_test_precision = test_precision;
				max_test_iterator = j;
			}
		}

		t4 = timeGetTime();
		cout << "Use Time:" << (t4 - t3)*1.0 / 1000 << endl;
		result << "Use Time:" << (t4 - t3)*1.0 / 1000 << endl;
		if (count >= exitor)
		{
			break;
		}
	}
	cout << train.name + "最大值是：" << "=" << max_train_precision << "在" << max_train_iterator << "次" << endl;
	cout << dev.name + "最大值是：" << "=" << max_dev_precision << "在" << max_dev_iterator << "次" << endl;
	result << train.name + "最大值是：" << "=" << max_train_precision << "在" << max_train_iterator << "次" << endl;
	result << dev.name + "最大值是：" << "=" << max_dev_precision << "在" << max_dev_iterator << "次" << endl;
	if (test.name.size() != 0)
	{
		cout << test.name + "最大值是：" << "=" << max_test_precision << "在" << max_test_iterator << "次" << endl;
		result << test.name + "最大值是：" << "=" << max_test_precision << "在" << max_test_iterator << "次" << endl;
	}
	t2 = timeGetTime();
	cout << "Use Time:" << (t2 - t1)*1.0 / 1000 << endl;
	result << "Use Time:" << (t2 - t1)*1.0 / 1000 << endl;
}

global_linear_model::global_linear_model(string &train_, string &dev_, string &test_)
{
	if (train_ != "")
	{
		train.read_data(train_);
	}
	if (dev_ != "")
	{
		dev.read_data(dev_);
	}
	if (test_ != "")
	{
		test.read_data(test_);
	}


}


global_linear_model::~global_linear_model()
{
}
