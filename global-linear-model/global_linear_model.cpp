#include "global_linear_model.h"
//实例化特征
vector<string> global_linear_model::create_feature(const sentence &sentence, int pos)
{
	string word = sentence.word[pos];//当前词。
	string former_tag;
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
	f.push_back("02:*" + word);
	f.push_back("03:*" + word_m1);
	f.push_back("04:*" + word_p1);
	f.push_back("05:*" + word + "*" + word_char_m1m1);
	f.push_back("06:*" + word + "*" + word_char_p1_first);
	f.push_back("07:*" + word_char_first);
	f.push_back("08:*" + word_char_last);
	int pos_word_len = sentence.word_char[pos].size();
	for (int k = 0; k < pos_word_len - 1; k++)
	{
		string cik = sentence.word_char[pos][k];
		f.push_back("09:*" + cik);
		f.push_back("10:*" + word_char_first + "*" + cik);
		f.push_back("11:*" + word_char_last + "*" + cik);
		string cikp1 = sentence.word_char[pos][k + 1];
		if (cik == cikp1)
		{
			f.push_back("13:*" + cik + "*" + "consecutive");
		}
	}
	if (pos_word_len == 1)
	{
		f.push_back("12:*" + word + "*" + word_char_m1m1 + "*" + word_char_p1_first);
	}
	for (int k = 0; k <pos_word_len; k++)
	{
		if (k >= 4)break;
		string prefix, suffix;
		//获取前缀
		for (int n = 0; n <= k; n++)
		{
			prefix = prefix + sentence.word_char[pos][n];
		}
		//获取后缀。
		for (int n = pos_word_len - k - 1; n <= pos_word_len - 1; n++)
		{
			suffix = suffix + sentence.word_char[pos][n];
		}
		f.push_back("14:*" + prefix);
		f.push_back("15:*" + suffix);
	}
	return f;
}
void global_linear_model::create_feature_space()
{
	train.read_data("train");
	dev.read_data("dev");
//	test.read_data("test");
	int word_count = 0, tag_count = 0;
	vector<string> f;
	for (auto sen = train.sentences.begin(); sen != train.sentences.end(); sen++)
	{
		for (int i = 0; i < sen->word.size(); i++)
		{
			if (i == 0)
			{
				f=create_feature(*sen, i);
				f.push_back("01:*&&");
			}
			else
			{
				f=create_feature(*sen, i);
				f.push_back("01:*" + sen->tag[i - 1]);
			}
			for (auto i = f.begin(); i != f.end(); i++)
			{
				if (model.find(*i) == model.end())
				{
					model[*i] = word_count;
					feature.push_back(*i);
					word_count++;
				}
			}
			f.clear();
			if (tag.find(sen->tag[i]) == tag.end())
			{
				tag[sen->tag[i]] = tag_count;
				vector_tag.push_back(sen->tag[i]);
				tag_count++;
			}
		}
	}
	w.reserve(tag.size()*model.size());
	for (int j = 0; j < tag.size()*model.size(); j++)
	{
		w.push_back(0);
	}
	vector<vector<int>> c(tag.size(), vector<int>(tag.size(), 0));
	for (int i = 0; i < tag.size(); i++)
	{
		for (int j = 0; j < tag.size(); j++)
		{
			if (model.find("01:*" + vector_tag[j]) != model.end())
			{
				int max_tag_id = model["01:*" + vector_tag[j]];
				c[i][j] = w[i*model.size() + max_tag_id];
			}
		}
	}
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
	  			feature_id.push_back(model[feature[i]]);
  		}
  	}
  	return feature_id;
}

vector<int> global_linear_model::count_score(const vector<string> &feature)
{
	int offset = 0;
	vector<vector<int>> scores(tag.size(), vector<int>(feature.size(), 0));
	for (int j = 0; j < tag.size(); j++)
	{
		offset = j*model.size();
		for (int i = 0; i < feature.size(); i++)
		{
			if (model.find(feature[i]) != model.end())
			{
				scores[j][i] += w[model[feature[i]] + offset];
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
void global_linear_model::update_w(const sentence &sen, const vector<string> &max_sentence_tag)
{
	if ((sen.tag[0] != max_sentence_tag[0]))
	{
		vector<string> feature=create_feature(sen, 0);
		feature.push_back("01:*&&");
		vector<int> fv_id=get_id(feature);
		int correct_offset = tag[sen.tag[0]] * model.size();
		int max_offset = tag[max_sentence_tag[0]] * model.size();
		for (int j = 0; j < fv_id.size(); j++)
		{
			int correct_index = correct_offset + fv_id[j];
			int max_index = max_offset + fv_id[j];
			w[correct_index] = w[correct_index] + 1;
			w[max_index] = w[max_index] - 1;
		}
	}
	for (int i = 1; i < sen.word.size(); i++)
	{
		if ((sen.tag[i] != max_sentence_tag[i]))
		{
			//实际构造的特征只有第一个不一样（即前面的词性），其他的都一样，所以只构造一个。//分开加减即可。
			vector<string> correct_feature = create_feature(sen, i);
			correct_feature.push_back("01:*" + sen.tag[i - 1]);
			vector<int> correct_fv_id=get_id(correct_feature);
			//处理第一部分的特征，正确的加。
			int correct_offset = tag[sen.tag[i]] * model.size();
			int correct_tag_index = correct_offset + model["01:*" + sen.tag[i - 1]];
			w[correct_tag_index]++;
			//错误的减。
			int max_offset = tag[max_sentence_tag[i]] * model.size();
			int max_tag_id = model["01:*" + max_sentence_tag[i - 1]];
			int max_tag_index = max_offset + max_tag_id;
			w[max_tag_index]--;
			//从02开始的特征都是一样的，不管正确的还是错误的。
			for (int j = 1; j < correct_fv_id.size(); j++)
			{
				int correct_index = correct_offset + correct_fv_id[j];
				w[correct_index]++;
				int max_index = max_offset + correct_fv_id[j];
				w[max_index]--;
			}
		}
	}
}
vector<string> global_linear_model::max_score_sentence_tag(const sentence &sen)
{
	vector<vector<int>> score_prob(sen.tag.size(), vector<int>(tag.size(), 0));
	vector<vector<int>> score_path(sen.tag.size(), vector<int>(tag.size(), 0));
	for (int i = 0; i < tag.size(); i++)
	{
		score_path[0][i] = -1;
	}
	//第一个词。
	vector <string> feature=create_feature(sen, 0);
	feature.push_back("01:*&&");
	score_prob[0] = count_score(feature);
	//其余的词。
	for (int j = 1; j < sen.word.size(); j++)
	{

			vector<int> currect_prob(tag.size(), 0);
			vector <string> feature=create_feature(sen, j);	
			vector<int> score = count_score(feature);
			for (int i = 0; i < tag.size(); i++)
			{
				for (int z = 0; z < tag.size(); z++)
				{
					currect_prob[z] = head_prob[i][z] + score_prob[j - 1][z];
				}
				auto w = max_element(begin(currect_prob), end(currect_prob));
				score_prob[j][i] = *w+score[i];
				score_path[j][i] = distance(begin(currect_prob), w);
			}
	}
	vector<string> path_max;
	vector<int> a = score_prob[sen.word.size() - 1];
	auto w = max_element(begin(a), end(a));
	int max_point = distance(begin(a),w);
	for (int j = sen.word.size() - 1; j > 0; j--)
	{
		path_max.push_back(vector_tag[max_point]);
		max_point = score_path[j][max_point];
	}
	path_max.push_back(vector_tag[max_point]);
	reverse(path_max.begin(), path_max.end());
	return path_max;
}
double global_linear_model::evaluate(dataset data)
{
	int correct = 0, all = 0;
	for (auto sen = data.sentences.begin(); sen!= data.sentences.end(); sen++)
	{
		vector<string> max_sentence_tag=max_score_sentence_tag(*sen);
		for (int i = 0; i < max_sentence_tag.size(); i++)
		{
			if (max_sentence_tag[i] == sen->tag[i])
			{
				correct++;
			}
			all++;
		}
	}
	cout << data.name << ":" << correct << "/" << all << "=" << (correct / double(all)) << endl;
	return (correct / double(all));
}
void global_linear_model::online_training()
{
	double max_train_precision = 0, max_dev_precision = 0, max_test_precision = 0;
	ofstream result("smallresult.txt");
	if (!result)
	{
		cout << "don't open feature file" << endl;
	}
	result << train.name << "共" << train.sentence_count << "个句子，共" << train.word_count << "个词" << endl;
	result << dev.name << "共" << dev.sentence_count << "个句子，共" << dev.word_count << "个词" << endl;
//	result << test.name << "共" << test.sentence_count << "个句子，共" << test.word_count << "个词" << endl;
	DWORD t1, t2, t3, t4;
	t1 = timeGetTime();
	for(int j=0;j<20;j++)
	{
		cout << "iterator" << j << endl;
		result << "iterator " << j << endl;
		t3 = timeGetTime();
		for (auto sen = train.sentences.begin(); sen != train.sentences.end(); sen++)
		{
				vector<string> max_sentence_tag=max_score_sentence_tag(*sen);
				if (max_sentence_tag != sen->tag)
				{
					update_w(*sen,max_sentence_tag);

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
		double train_precision=evaluate(train);
		double dev_precision = evaluate(dev);
	//	double test_precision = evaluate(test);
		result << "train" << train_precision << endl;
		result << "dev" << dev_precision << endl;
//		result << "test" << test_precision << endl;
		t4 = timeGetTime();
		cout << "Use Time:" << (t4 - t3)*1.0 / 1000 << endl;
		result << "Use Time:" << (t4 - t3)*1.0 / 1000 << endl;
		if (train_precision > max_train_precision)
		{
			max_train_precision = train_precision;
		}
		if (dev_precision > max_dev_precision)
		{
			max_dev_precision = dev_precision;
		}
		/*
		if (test_precision > max_test_precision)
		{
			max_test_precision = test_precision;
		}
		*/

	}	
	cout << train.name << "=" << max_train_precision << endl;
	cout << dev.name << "=" << max_dev_precision << endl;
//	cout << test.name << "=" << max_test_precision << endl;
	result << train.name + "最大值是:" << "=" << max_train_precision << endl;
	result << dev.name + "最大值是:" << "=" << max_dev_precision << endl;
//	result << test.name + "最大值是:" << "=" << max_test_precision << endl;
	t2 = timeGetTime();
	cout << "Use Time:" << (t2 - t1)*1.0 / 1000 << endl;
	result << "Use Time:" << (t2 - t1)*1.0 / 1000 << endl;
}

global_linear_model::global_linear_model()
{
}


global_linear_model::~global_linear_model()
{
}
