#include <iostream>
#include <vector>
#include <string>

// #pragma OPENCL EXTENSION cl_intel_printf : enable
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.hpp>
#include <cxxopts.hpp>

#include <common/Data.hpp> // Data
#include <common/DataGenerator.hpp> // DataGenerator
#include <common/Timer.hpp> // Timer

#include "Utils.hpp"
#include "IntelCPUParallelCopa.hpp"

void printCLSDK() {
	std::vector<cl::Platform> platforms;
	std::vector<cl::Device> devices;
	cl::Platform::get(&platforms);
	if (platforms.empty()) {
		std::cerr << "OpenCL is not installed." << std::endl;
		return;
	}
	std::cout << "Platforms\n";
	std::cout << "------------------\n";
	for (int32_t i = 0; i < platforms.size(); ++i) {
		std::cout << "[" << i << "] " << platforms[i].getInfo<CL_PLATFORM_NAME>() << "\n";
		platforms[i].getDevices(CL_DEVICE_TYPE_ALL, &devices);
		for (int32_t i = 0; i < devices.size(); ++i) {
			std::cout << "  - [" << i << "] " << devices.at(i).getInfo<CL_DEVICE_NAME>() << "\n";
		}
	}
}

int main(int argc, char** argv) {
	int device_idx = 0, platform_idx = 0, n = 10, num_threads = 1, seed=0;
	std::string cl_file_path;
	cxxopts::Options options(argv[0], "Solve kanpsack problem on heterogeous cores using OpenCL");
	options.add_options()
		("d,device", "Device Index", cxxopts::value<int>(device_idx)->default_value("0"))
		("p,platform", "Platform Index", cxxopts::value<int>(platform_idx)->default_value("0"))
		("n,", "The number of data", cxxopts::value<int>(n)->default_value("10"))
		("j,", "the number of threads", cxxopts::value<int>(num_threads)->default_value("8"))
		("s,seed", "Seed for data generator", cxxopts::value<int>(seed)->default_value("0"))
		("f,cl_file_path", "path of copa_kernels.cl file", cxxopts::value<std::string>(cl_file_path)->default_value("copa_kernels.cl"))
		("list", "List platforms and devices")
		("h,help", "Print help")
		;
	
	try
	{
		auto result = options.parse(argc, argv);
		if (result.count("list")) 
		{
			printCLSDK();
			return EXIT_SUCCESS;
		}
		if (result.count("h") || result.count("help")) 
		{
			std::cout << options.help() << "\n";
			return EXIT_SUCCESS;
		}
	}
	catch (const cxxopts::OptionException& e)
	{
		std::cerr << "ERROR: parsing options: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	

	

	std::vector<cl::Platform> platforms;
	std::vector<cl::Device> devices;
	cl::Platform::get(&platforms);
	if (platforms.empty()) {
		std::cerr << "OpenCL is not installed." << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << "Selected Platform: " << platforms.at(platform_idx).getInfo<CL_PLATFORM_NAME>() << "\n";
	platforms.at(platform_idx).getDevices(CL_DEVICE_TYPE_ALL, &devices);
	std::cout << "Selected Device:   " << devices.at(1).getInfo<CL_DEVICE_NAME>() << "\n";
	Data data = DataGenerator::generate(n, seed);
	device_idx = 1;

	cl_long solution = 0;
	std::bitset<64> solutionSet;
	int64_t time=0;
	try {
		IntelCPUParallelCopa copa(platform_idx, device_idx, num_threads,
			data, cl_file_path);
		solution = copa.Solve();
		solutionSet = copa.getSolutionSet();
		time = copa.getElapsedTime();
	}
	catch (const CLFileNotExistException& e)
	{
		std::cerr << e.what() << "\n";
		return EXIT_FAILURE;
	}

	std::cout << "Solution:     " << solution << "\n";
	std::cout << "Solution Set: " << solutionSet.to_string().substr(64-n, n) << "\n";
	std::cout << "Elapsed Time: " << time << "ns\n";

	return EXIT_SUCCESS;
}