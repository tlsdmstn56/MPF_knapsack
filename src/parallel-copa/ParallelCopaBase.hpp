#pragma once

#include <CL/cl.hpp>
#include <common/Data.hpp>

#include "Utils.hpp"


class ParallelCopaBase {
public:
	ParallelCopaBase(const int platform_idx, const int device_idx, int num_work_item, int num_compute_unit, const Data& data) :
		num_work_item(num_work_item), data(data), num_compute_unit(num_compute_unit) {
		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);
		platform = platforms.at(platform_idx);
		std::vector<cl::Device> devices;
		platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
		device = devices.at(device_idx);
		std::cout << "Selected Platform: " << platform.getInfo<CL_PLATFORM_NAME>() << "\n";
		std::cout << "Selected Device:   " << device.getInfo<CL_DEVICE_NAME>() << "\n";
		std::cout << "Working Group Size:   " << num_work_item << "\n";
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
	const cl_int num_work_item;
	const cl_int num_compute_unit;
	const Data data;
	int64_t elapsed_time;
	int64_t solution;
};