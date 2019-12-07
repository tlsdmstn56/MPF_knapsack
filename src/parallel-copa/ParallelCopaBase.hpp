#pragma once

#include <CL/cl.hpp>
#include <common/Data.hpp>

#include "Utils.hpp"


class ParallelCopaBase {
public:
	ParallelCopaBase(const int platform_idx, const int device_idx, int num_parallel, const Data& data) :
		num_parallel(num_parallel), data(data) {
		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);
		platform = platforms.at(platform_idx);
		std::vector<cl::Device> devices;
		platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
		device = devices.at(device_idx);
		context = cl::Context(device);
		queue = cl::CommandQueue(context, device);
	}
	virtual int64_t Solve() = 0;
	int64_t GetSolution() const noexcept {
		return solution;
	}
	virtual ~ParallelCopaBase() = default;
protected:
	cl::Platform platform;
	cl::Device device;
	cl::Context context;
	cl::CommandQueue queue;
	const cl_long num_parallel;
	const Data data;
	int64_t elapsed_time;
	int64_t solution;
};