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
#include <common/VariadicTable.hpp>

#include "Utils.hpp"
#include "IntelCPUParallelCopa.hpp"

using namespace std;

void printCLSDK() {
	vector<cl::Platform> platforms;
	vector<cl::Device> devices;
	cl::Platform::get(&platforms);
	if (platforms.empty()) {
		cerr << "OpenCL is not installed." << endl;
		return;
	}
	VariadicTable<string, string, cl_uint, size_t,cl_ulong, cl_ulong, cl_uint, cl_ulong> vt({
		"Platform", "Device", "Max CU size", "Max WG Size","Max Globel Mem", "Global Mem Cache Size",
		"Global Mem Cacheline Size", "Local Mem Size" });
	for (int32_t i = 0; i < platforms.size(); ++i) {
		string plaform = "[" + to_string(i) + "] " + platforms[i].getInfo<CL_PLATFORM_NAME>();
		platforms[i].getDevices(CL_DEVICE_TYPE_ALL, &devices);
		for (int32_t i = 0; i < devices.size(); ++i) {
			string device = "[" + to_string(i) + "] " + devices.at(i).getInfo<CL_DEVICE_NAME>();
			cl_uint max_cu = devices.at(i).getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
			size_t max_wg = devices.at(i).getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE >();
			cl_ulong global_mem = devices.at(i).getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>();
			cl_ulong global_mem_cache = devices.at(i).getInfo<CL_DEVICE_GLOBAL_MEM_CACHE_SIZE>();
			cl_uint	 global_mem_cacheline = devices.at(i).getInfo<CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE>();
			cl_ulong  local_mem = devices.at(i).getInfo<CL_DEVICE_LOCAL_MEM_SIZE>();
			vt.addRow({ plaform, device, max_cu, max_wg, global_mem, global_mem_cache, global_mem_cacheline, local_mem});
		}
	}
	vt.print(std::cout);
}

bool isNumThreadsValid(int num_work_item, int n)
{
	return ((1 << (n / 2)) % num_work_item) == 0;
}

int main(int argc, char** argv) {
	int device_idx = 0, platform_idx = 0, n = 10, num_work_item = 1, seed=0;
	string cl_file_path;
	cxxopts::Options options(argv[0], "Solve kanpsack problem on heterogeous cores using OpenCL");
	options.add_options()
		("p,platform", "Platform Index", cxxopts::value<int>(platform_idx)->default_value("0"))
		("d,device", "Device Index", cxxopts::value<int>(device_idx)->default_value("0"))
		("wi,", "the number of work items per a group", cxxopts::value<int>(num_work_item)->default_value("1"))
		("n,", "The number of data", cxxopts::value<int>(n)->default_value("10"))
		("s,seed", "Seed for data generator", cxxopts::value<int>(seed)->default_value("0"))
		("f,cl_file_path", "path of copa_kernels.cl file", cxxopts::value<string>(cl_file_path)->default_value("src/parallel-copa/copa_kernels.cl"))
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
			cout << options.help() << "\n";
			return EXIT_SUCCESS;
		}
	}
	catch (const cxxopts::OptionException& e)
	{
		cerr << "ERROR: parsing options: " << e.what() << endl;
		return EXIT_FAILURE;
	}
	if (!isNumThreadsValid(num_work_item, n))
	{
		cerr << "ERROR: num_work_item must be power of 2" << endl;
		return EXIT_FAILURE;
	}

	

	vector<cl::Platform> platforms;
	vector<cl::Device> devices;
	cl::Platform::get(&platforms);
	if (platforms.empty()) {
		cerr << "OpenCL is not installed." << endl;
		return EXIT_FAILURE;
	}
	
	// FIXME:
	n = 10; seed = 0;
	platform_idx = 0; 
	device_idx = 1;
	num_work_item = 32;
	Data data = DataGenerator::generate(n, seed);

	cl_int solution = 0;
	bitset<64> solutionSet;
	int64_t time=0;
	try {
		IntelCPUParallelCopa copa(platform_idx, device_idx, num_work_item,
			data, cl_file_path);
		solution = copa.Solve();
		solutionSet = copa.getSolutionSet();
		time = copa.getElapsedTime();
	}
	catch (const CLFileNotExistException& e)
	{
		cerr << e.what() << "\n";
		return EXIT_FAILURE;
	}

	cout << "Solution:     " << solution << "\n";
	cout << "Solution Set: " << solutionSet.to_string().substr(64-n, n) << "\n";
	cout << "Elapsed Time: " << time/1000000ll << " ms\n";

	return EXIT_SUCCESS;
}