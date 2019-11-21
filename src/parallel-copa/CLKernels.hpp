#pragma once

// #include <algorithm>
#include <list>
#include <vector>
#include <iterator>
#include <functional>
#include <utility>

struct Triple
{
	int64_t set;				//Set denotes which Elements are in the Subset
	double w;					//Weight of the Triple
	double p;					//Profit of the Triple

	//Struct Constructor
	Triple() : set(0), w(0.0), p(0.0) { }

	//Comparison Operator Overloadings
	constexpr bool operator< (const Triple &t) const noexcept
	{ return (w < t.w); }

	constexpr bool operator> (const Triple &t) const noexcept
	{ return (w > t.w); }
};

void MergeLists(std::vector<Triple> &A, std::vector<Triple> &B,
					std::vector< std::pair<double, double> > &V)
{
	std::vector<Triple> T_p, Tcopy;
	Triple t;

	int64_t v1s = V.size() >> 1, v2s = V.size() - v1s;

	//Initialisation for A
	t.set = 0, t.w = t.p = 0;
	A.push_back(t);

	//Sort A in Non-Increasing Order
	for (int64_t i = 0; i < v1s; ++i)
	{
		T_p.clear();
		Tcopy.clear();

		//Add Elements to Subset (Triple) ti
		//Add ti to T_p
		for (int64_t j = 0; j < (int64_t)A.size(); ++j)
		{
			t.set = A[j].set + (1ll << i);
			t.w = A[j].w + V[i].first;
			t.p = A[j].p + V[i].second;

			T_p.push_back(t);
		}

		//Merge A, T_p
		merge(A.begin(), A.end(), T_p.begin(), T_p.end(), std::back_inserter(Tcopy));
		A = Tcopy;
	}

	//Initialisation for B
	t.set = 0, t.w = t.p = 0;
	B.push_back(t);

	//Sort B in Non-Increasing Order
	for (int64_t i = 0; i < v2s; ++i)
	{
		T_p.clear();
		Tcopy.clear();

		//Add Elements to Subset (Triple) ti
		//Add ti to T_p
		for (int64_t j = 0; j < (int64_t)B.size(); ++j)
		{
			t.set = B[j].set + (1ll << i);
			t.w = B[j].w + V[i + v1s].first;
			t.p = B[j].p + V[i + v1s].second;

			T_p.push_back(t);
		}

		//Merge B, T_p
		merge(B.begin(), B.end(), T_p.begin(), T_p.end(), std::back_inserter(Tcopy), std::greater<>());
		B = Tcopy;
	}
}

void MaxScan(std::vector<Triple> &B, std::vector< std::pair<double, int64_t> > &maxB)
{
	int64_t Bsize = B.size();
	maxB[Bsize - 1].first = B[Bsize - 1].p;
	maxB[Bsize - 1].second = Bsize - 1;
	for (int64_t i = Bsize - 2; i >= 0; i--)
	{
		if (B[i].p>maxB[i + 1].first)
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

int64_t GenerateSets(std::vector<Triple> &A, std::vector<Triple> &B, const double &c,
						std::vector< std::pair<double, int64_t> > &maxB, int64_t N)
{
	double bestValue = 0;
	std::pair<int64_t, int64_t> bestSet;

	int64_t i = 0, j = 0;
	while(i < N && j < N)
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
			bestSet = std::make_pair(A[i].set, maxB[j].second);
		}
		++i;
	}

	return static_cast<int64_t>(bestValue);
}

int64_t DPKnapsack(int64_t W, double wt[], double val[], int64_t n)
{
	int64_t i, w;
	std::vector< std::vector<double> > K(n + 1, std::vector<double>(W + 1));

	// Build table K[][] in bottom up manner 
	for (i = 0; i <= n; i++)
	{
		for (w = 0; w <= W; w++)
		{
			if (i == 0 || w == 0)
				K[i][w] = 0;
			else if (wt[i - 1] <= w)
				K[i][w] = max(val[i - 1] + K[i - 1][w - static_cast<int64_t>(wt[i - 1])], 
					K[i - 1][w]);
			else
				K[i][w] = K[i - 1][w];
		}
	}
	return static_cast<int64_t>(K[n][W]);
}