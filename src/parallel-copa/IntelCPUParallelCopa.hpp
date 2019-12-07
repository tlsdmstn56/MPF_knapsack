#pragma once

#include <algorithm>
#include <iostream>
#include <vector>
#include <bitset>

#include <CL/cl.h>

#include <common/Timer.hpp>
#include "Utils.hpp"
#include "ParallelCopaBase.hpp"
#include "Triple.hpp"
#include "Pair.hpp"

class IntelCPUParallelCopa : protected ParallelCopaBase
{
public:
	IntelCPUParallelCopa(const int platform_idx, const int device_idx, int num_parallel, const Data& data, const std::string& cl_file_path) :
		ParallelCopaBase(platform_idx, device_idx, num_parallel, data), cl_file_path(cl_file_path) { }
	int64_t Solve() override;
	virtual ~IntelCPUParallelCopa()
	{
		queue.finish();
	}
	constexpr std::bitset<64> getSolutionSet() const noexcept {
		return solution_set;
	}
	constexpr int64_t getElapsedTime() const noexcept {
		return elapsedTime;
	}

private:
	Timer timer;
	std::string cl_file_path;
	cl::Program program;
	std::vector<Triple> A;
	std::vector<Triple> B;

	// buffers
	cl::Buffer buffer_A;
	cl::Buffer buffer_B;
	cl::Buffer buffer_AMaxValues;
	cl::Buffer buffer_BMaxValues;
	cl::Buffer buffer_pruned;
	cl::Buffer buffer_max;
	cl::Buffer buffer_max_val;
	cl::Buffer buffer_max_val_pair;

	// split A and B
	//  - sort A non-decreasing
	//  - sort B non-increasing
	void split_vector() noexcept;
	// stage 1
	void parallel_generation();
	// stage 2
	void first_max_scan();
	// stage 3
	void prune();
	// stage 4
	void second_max_scan();
	// stage 5
	void final_search();

	cl_long ASize = 0;
	cl_long BSize = 0;
	cl_long A_local_size = 0;
	cl_long B_local_size = 0;
	cl_long A_buffer_size = 0;
	cl_long B_buffer_size = 0;

	cl_long solution_value = 0;
	std::bitset<64> solution_set;

	void printError(cl_int err, const std::string& msg) const {
		if (err != CL_SUCCESS) {
			std::cout << msg << "\n";
		}
	}

	template<typename T>
	void printBuffer(const cl_long size, const cl::Buffer& buffer, const std::string& name) const {
		T* t = new T[size]();
		queue.enqueueReadBuffer(buffer, CL_TRUE, 0, size * sizeof(T), t);
		std::cout << name << "\n";
		for (size_t i = 0, end = size; i < end; ++i)
		{
			const T& ent = t[i];
			std::cout << ent << "\n";
		}
		delete[] t;
	}
	template<>
	void printBuffer<Triple>(const cl_long size, const cl::Buffer& buffer, const std::string& name) const {
		Triple* t = new Triple[size]();
		queue.enqueueReadBuffer(buffer, CL_TRUE, 0, size * sizeof(Triple), t);
		std::cout << name << "\n";
		for (size_t i = 0, end = size; i < end; ++i)
		{
			const auto& ent = t[i];
			std::cout << ent.w << " " << ent.p << " " << ent.set << "\n";
		}
		delete[] t;
	}
	template<>
	void printBuffer<Pair>(const cl_long size, const cl::Buffer& buffer, const std::string& name) const {
		Pair* t = new Pair[size]();
		queue.enqueueReadBuffer(buffer, CL_TRUE, 0, size * sizeof(Pair), t);
		std::cout << name << "\n";
		for (size_t i = 0, end = size; i < end; ++i)
		{
			const auto& ent = t[i];
			std::cout << ent.a_idx << " " << ent.b_idx << "\n";
		}
		delete[] t;
	}
	int64_t elapsedTime=0;
};