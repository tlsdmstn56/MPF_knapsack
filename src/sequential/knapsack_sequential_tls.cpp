#include <iostream>
#include <algorithm>
#include <list>
#include <vector>
#include <iterator>
#include <functional>
#include <time.h>
#include <chrono>
#include <cstdlib>

#include <cxxopts.hpp>

#include <common/DataGenerator.hpp>
#include <common/Data.hpp>
#include <common/Timer.hpp>



using namespace std;

struct Triple
{
	int64_t set;				//Set denotes which Elements are in the Subset
	int64_t w;					//Weight of the Triple
	int64_t p;					//Profit of the Triple
	Triple() : set(0),
		w(0.0),
		p(0.0)
	{}
	bool operator< (const Triple& t) const
	{
		return (w < t.w);
	}

	bool operator> (const Triple& t) const
	{
		return (w > t.w);
	}
};

void MergeLists(vector<Triple>& A, vector<Triple>& B,
	vector< pair<int64_t, int64_t> >& V)
{
	vector<Triple> T_p, Tcopy;
	Triple t;

	int64_t v1s = V.size() >> 1, v2s = V.size() - v1s;

	t.set = 0, t.w = t.p = 0;
	A.push_back(t);


	for (int64_t i = 0; i < v1s; ++i)
	{
		T_p.clear();
		Tcopy.clear();
		for (int64_t j = 0; j < (int64_t)A.size(); ++j)
		{
			t.set = A[j].set + (1 << i);
			t.w = A[j].w + V[i].first;
			t.p = A[j].p + V[i].second;

			T_p.push_back(t);
		}
		merge(A.begin(), A.end(), T_p.begin(), T_p.end(), back_inserter(Tcopy));
		A = Tcopy;
	}
	t.set = 0, t.w = t.p = 0;
	B.push_back(t);
	for (int64_t i = 0; i < v2s; ++i)
	{
		T_p.clear();
		Tcopy.clear();
		for (int64_t j = 0; j < (int64_t)B.size(); ++j)
		{
			t.set = B[j].set + (1 << i);
			t.w = B[j].w + V[i + v1s].first;
			t.p = B[j].p + V[i + v1s].second;

			T_p.push_back(t);
		}
		merge(B.begin(), B.end(), T_p.begin(), T_p.end(), back_inserter(Tcopy), greater<>());
		B = Tcopy;
	}
}

void MaxScan(vector<Triple>& B, vector< pair<int64_t, int64_t> >& maxB)
{
	int64_t Bsize = B.size();
	maxB[Bsize - 1].first = B[Bsize - 1].p;
	maxB[Bsize - 1].second = Bsize - 1;
	for (int64_t i = Bsize - 2; i >= 0; i--)
	{
		if (B[i].p > maxB[i + 1].first)
		{
			maxB[i].first = B[i].p;
			maxB[i].second = i;
		}
		else
		{
			maxB[i].first = maxB[i + 1].first;
			maxB[i].second = maxB[i + 1].second;
		}
	}
}

int64_t GenerateSets(vector<Triple>& A, vector<Triple>& B, const int64_t& c,
	vector< pair<int64_t, int64_t> >& maxB, int64_t N)
{
	int64_t bestValue = 0;
	pair<int64_t, int64_t> bestSet;

	int64_t i = 0, j = 0;
	while (i < N && j < N)
	{
		if (A[i].w + B[j].w > c)
		{
			++j;

			if (j == N) break;
			else continue;
		}

		if (A[i].p + maxB[j].first > bestValue)
		{
			bestValue = A[i].p + maxB[j].first;
			bestSet = make_pair(A[i].set, maxB[j].second);
		}
		++i;
	}

	return bestValue;
}

int main(int argc, char** argv)
{
	int n = 10, seed = 0, repeat = 1;
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
	Data data = DataGenerator::generate(n, seed);
	int c = data.getCapacity();
	auto V = data.getTable();	
	Timer timer;
	timer.start();
	vector<Triple> A, B;
	MergeLists(A, B, V);
	vector< pair<int64_t, int64_t> > maxB((int64_t)B.size());
	MaxScan(B, maxB);
	int64_t solutoin = GenerateSets(A, B, c, maxB, (1 << (int64_t)(V.size() >> 1)));
	int64_t elapsed_time = timer.stop();
	cout << "Solution: " << solutoin << "\n";
	cout << "Elapsed Time: " << (double)elapsed_time / 1000000.0 << " ms\n";
	return 0;
}