#include <iostream>
#include <vector>
#include <chrono>
#include <string>

#include <cxxopts.hpp>

#include <common/DataGenerator.hpp>
#include <common/Data.hpp>
#include <common/Timer.hpp>

using namespace std;

int main(int argc, char** argv){
	int n = 40, seed = 0, repeat = 1;
	std::string out_csv;
	cxxopts::Options options(argv[0], "Solve kanpsack problem on single core (DP)");
	options.add_options()
		("n,data_size", "The number of data", cxxopts::value<int>(n)->default_value("10"))
		("s,seed", "Seed for data generator", cxxopts::value<int>(seed)->default_value("0"))
		("h,help", "Print help")
		;
	try
	{
		auto result = options.parse(argc, argv);
		if (result.count("h") || result.count("help"))
		{
			cout << options.help() << "\n";
			return EXIT_SUCCESS;
		}
	}
	catch (const cxxopts::OptionException& e)
	{
		cerr << "ERROR: parsing options: " << e.what() << endl;
		return EXIT_FAILURE;
	}
	cout << "N: " << n << ", seed: " << seed << "\n";
	const Data data = DataGenerator::generate(n, seed);
	const auto table = data.getTable();
	
	const int W = data.getCapacity();
	
	vector<int> wt;
	vector<int> val;
	for (auto& a : table) {
		wt.push_back(a.first);
		val.push_back(a.first);
	}
	
	Timer timer;
	timer.start();
	vector<vector<int>> K;
	for (int i = 0; i <= n; ++i) {
		K.push_back(vector<int>(W + 1, 0));
	}
	for (int i = 0; i <= n; i++)
	{
		for (int w = 0; w <= W; w++)
		{
			if (i == 0 || w == 0)
				K[i][w] = 0;
			else if (wt[i - 1] <= w) {
				int tmp = val[i - 1] + K[i - 1][w - wt[i - 1]];
				K[i][w] = (tmp > K[i - 1][w]) ? tmp : K[i - 1][w];
			}
			else
				K[i][w] = K[i - 1][w];
		}
	}
	int64_t elapsed_time = timer.stop();
	cout << "Solution: " << K[n][W] << "\n";
	cout << "Elapsed Time: " << (double)elapsed_time/ 1000000.0 << " ms\n";
	return 0;
}