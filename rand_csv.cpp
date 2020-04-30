#include <iostream>
#include <fstream>
#include<vector>
#include <algorithm>
#include <iomanip>
#include <set>
#include<random>
#include <chrono>
#include<string>

#define NEEDED_NUM 5

using namespace std;


class DataGenerator {
public:
	const int SUM_LIM_LOW = 16362411;
	const int SUM_LIM_HIGH = 17036971;
	const double _D_LOW = 0.0;
	DataGenerator(string path)
	{
		load(path);
		d_num= field_b.size();

	}


	vector<double> field_b;//量级
	vector<double> field_c;//最大值
	vector<double> field_d;//随机值

	int d_num;

	mt19937* p_engine;
	void setEngine(mt19937* p)
	{
		p_engine = p;
	}

	void load(string &path)
	{
		ifstream _csvInput;
		_csvInput.open(path);



		for (int i = 0; i < 747; i++)
		{
			string strs;

			_csvInput >> strs;
			int loc = 0;
			while (strs[loc] != ',')
				loc++;
			string s1 = strs.substr(0, loc);
			field_b.push_back(atof(s1.c_str()));

			loc++;
			int loc2 = loc;
			while (strs[loc] != ',')
				loc++;
			string s2 = strs.substr(loc2, loc - loc2);
			field_c.push_back(atof(s2.c_str()));

		}

		d_num = field_b.size();
		//field_d.resize(rowsNum);
	}
	
	vector<double> random_d()
	{
		vector<double> res;
		for (auto i : field_c)
		{
			res.push_back(i* random_0_1());
		}
		return res;
	}

	void mutation_d(int target)
	{
		int _loss = loss_fn(target);
		bool _loss_pos = _loss > 0;
		double percentage = abs(double(_loss)/target);

		int mutation_num = percentage < 1.0 ? (percentage*d_num): d_num;

		if (mutation_num < 100)
			mutation_num += 100;

		vector<double> sample_probs;
		int sum_b_d = sum_bd();
		for (int i = 0; i < d_num;++i)
		{
			sample_probs.push_back(field_b[i]*field_d[i]/sum_b_d);
		}

		//当前的(随机值-目标值)为负值时
		if (!_loss_pos)
		{
			double sum_probs = 0.0;
			for (auto prob : sample_probs)
			{
				sum_probs += (1 - prob);
			}

			for (int i=0;i<sample_probs.size();++i)
			{
				sample_probs[i] = (1 - sample_probs[i]) / sum_probs;
			}
		}

		set<int> idx_list;
		idx_list = random_choice(d_num,mutation_num,sample_probs);



		for (auto i : idx_list)
		{
			int _low, _high;
			if (_loss_pos)
			{
				_low = _D_LOW;
				_high = field_d[i];
			}
			else
			{
				_low = field_d[i];
				_high = field_c[i];
			}

			field_d[i] = random_0_1() * (_high - _low) + _low;
		}


	}

	int sum_bd()//field_d dot field_b
	{
		long res = 0;
		for (int i = 0; i < field_b.size(); ++i)
		{
			res += field_b[i] * field_d[i];
		}
		return (int)res;
	}

	int loss_fn(int target)
	{
		return sum_bd()-target;
	}

	void init_rand_d()
	{
		field_d = random_d();
	}

	bool check_condition(int target,vector<int> loss_limits)
	{
		int val = loss_fn(target);
		return val>=loss_limits[0]&&val<=loss_limits[1];
	}

	float random_0_1()//0-1随机小数的随机精度0.001
	{
		return (rand() % 1000) * 0.001;
	}

	//按照p的概率生成size个元素，选择的范围为[0,max_idx],产生的元素不重复
	set<int> random_choice(int max_idx,int size,vector<double> &p)
	{
		set<int> S;
		discrete_distribution<size_t> dist(p.begin(),p.end());
		
		while (S.size() < size)
		{
			S.insert(dist(*p_engine));
		}

		return S;
	}

	void output_file(vector<vector<double>> &res,string file_name)
	{
		ofstream oFile;
		//打开要输出的文件
		oFile.open(file_name, ios::out | ios::trunc);
		for(auto one:res)
			for (auto e : one)
			{
				oFile << setw(6) << setfill('0') << setiosflags(ios::fixed) << setprecision(2) << e << ",";
				oFile << endl;
			}
				
		oFile.close();
	}
};


int main()
{
	vector<vector<double>> res;

	DataGenerator DG("1.csv");
	int MAX_SEED_NUM = 100;
	int	MAX_INIT_NUM = 1000;
	int	MAX_ITER_NUM = 1000;

	//迭代次数100*1000*1000
	int length = DG.SUM_LIM_HIGH - DG.SUM_LIM_LOW+1;
	srand(time(NULL));
	mt19937 engine;
	DG.setEngine(&engine);
	for (int t = 0; t < MAX_SEED_NUM; ++t)
	{
		float num;
		num = (rand() % 1000) * 0.001;

		int target = num * length + DG.SUM_LIM_LOW;
		vector<int> loss_limits(2);
		loss_limits[0] = target - DG.SUM_LIM_LOW;
		loss_limits[1] = DG.SUM_LIM_HIGH - target;
		cout << target << endl;

		for (int i = 0; i < MAX_INIT_NUM; ++i)
		{
			DG.init_rand_d();
			for (int j = 0; j < MAX_ITER_NUM; ++j)
			{
				DG.mutation_d(target);

				if (DG.check_condition(target, loss_limits))
				{
					res.push_back(DG.field_d);
					if (res.size() >= NEEDED_NUM)
					{
						DG.output_file(res, to_string(NEEDED_NUM)+"final.csv");
						return 1;
					}
					break;
				}
			}
		}
	}

	DG.output_file(res, to_string(res.size())+"final.csv");
	return 0;
}

