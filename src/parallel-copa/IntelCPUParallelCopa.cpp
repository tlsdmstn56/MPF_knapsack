#include "IntelCPUParallelCopa.hpp"
#include <algorithm>
#include <vector>
#include <bitset>
#include <CL/cl.hpp>

#include <common/Timer.hpp>

#include "Pair.hpp"



int64_t IntelCPUParallelCopa::Solve()
{
	timer.start();
	// Split A and B, sort
	split_vector();

	// setup opencl
	const auto src = GetSourceFromFile(cl_file_path);
	auto sources = cl::Program::Sources(1, std::make_pair(src.c_str(), src.length() + 1));
	program = cl::Program(context, sources);

	if (program.build({ device }, "-D __linux__ -cl-std=CL1.2") != CL_SUCCESS)
	{
		std::cout << "Error log:\n" << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
		exit(EXIT_FAILURE);
	}

	// init constants
	ASize = A.size();
	BSize = B.size();
	A_buffer_size = (1ll << ASize);
	B_buffer_size = (1ll << BSize);
	A_local_size = num_work_item;
	B_local_size = num_work_item;

	// init buffers
	cl_int err;
	buffer_A = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(Triple) * A_buffer_size, NULL, &err);
	printIfError(err, "buffer_A");
	buffer_B = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(Triple) * B_buffer_size, NULL, &err);
	printIfError(err, "buffer_B");

	// stage 1: parallel generation
	parallel_generation2();
	// stage 2: first max scan
	first_max_scan();
	// stage 3: prune
	prune();
	// stage 4: second max scan
	second_max_scan();
	// stage 5: final_search
	final_search();

	elapsedTime = timer.stop();

	return solution_value;
}



void IntelCPUParallelCopa::split_vector() noexcept
{
	const size_t ASize = (data.getTable().size() >> 1);
	const size_t BSize = data.getTable().size() - ASize;
	A.reserve(ASize);
	B.reserve(BSize);
	for (size_t i = 0; i < ASize; ++i)
	{
		const auto& entry = data.getTable().at(i);
		A.emplace_back(1ll << i, entry.first, entry.second);
	}
	for (size_t i = ASize; i < data.getTable().size(); ++i)
	{
		const auto& entry = data.getTable().at(i);
		B.emplace_back(1ll << i, entry.first, entry.second);
	}
	std::sort(A.begin(), A.end(), [](const Triple& a, const Triple& b) { return a.w < b.w; });
	std::sort(B.begin(), B.end(), [](const Triple& a, const Triple& b) { return a.w > b.w; });
}

void IntelCPUParallelCopa::parallel_generation2() {
	cl_int err;
	// buffer init
	cl::Buffer buffer_tmp = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(Triple) * (1ll << ((ASize > BSize ? ASize : BSize) - 1)), NULL, &err);
	printIfError(err, "buffer_tmp");
	cl::Buffer buffer_tmp2 = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(Triple) * (1ll << ((ASize > BSize ? ASize : BSize) - 1)), NULL, &err);
	printIfError(err, "buffer_tmp2");
	cl::Buffer buffer_arg = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(Triple), NULL, &err);
	printIfError(err, "buffer_arg");
	cl::Buffer buffer_quad = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(Quad)*num_compute_unit*2, NULL, &err);
	printIfError(err, "buffer_quad");
	// initial values
	const Triple init{ 1, 0, 0 };
	err = queue.enqueueWriteBuffer(buffer_A, CL_FALSE, 0, sizeof(Triple), &init);
	printIfError(err, "enqueue initial value to buffer_A");
	err = queue.enqueueWriteBuffer(buffer_B, CL_FALSE, 0, sizeof(Triple), &init);
	printIfError(err, "enqueue initial value to buffer_B");
	// define common kernels
	//     add_triple
	cl::Kernel add_triple(program, "add_triple", &err);
	printIfError(err, "add_triple kernel error");
	add_triple.setArg(0, buffer_tmp);
	add_triple.setArg(1, buffer_tmp2);
	add_triple.setArg(2, buffer_arg);
	// set logN
	int logN = -1;
	cl_int _num_compute_unit = num_compute_unit;
	while (_num_compute_unit) {
		_num_compute_unit >>= 1;
		++logN;
	}

	///////////////////////////////////////
	// Generate A array
	///////////////////////////////////////
	//     step1point2
	cl::Kernel step_1_2(program, "Step1Point2", &err);
	printIfError(err, "Step1Point2 kernel error");
	step_1_2.setArg(0, buffer_quad);
	step_1_2.setArg(1, buffer_tmp);
	step_1_2.setArg(2, buffer_tmp2);
	//     step2
	cl::Kernel step_2(program, "Step2", &err);
	printIfError(err, "Step2 kernel error");
	step_2.setArg(0, buffer_tmp);
	step_2.setArg(1, buffer_tmp2);
	step_2.setArg(2, buffer_A);
	step_2.setArg(3, buffer_quad);
	std::vector<Quad> quad_inits;
	for (int i = 0; i < ASize; ++i)
	{
		const cl_int size = (1ll << i);
		const cl_int bytesize = sizeof(Triple) * size;
		queue.enqueueCopyBuffer(buffer_A, buffer_tmp, 0, 0, bytesize);
		queue.enqueueWriteBuffer(buffer_arg, CL_FALSE, 0, sizeof(Triple), &(A.at(i)));
		queue.enqueueNDRangeKernel(add_triple,
			cl::NullRange,
			cl::NDRange(size),
			cl::NullRange);
		
		quad_inits.emplace_back(0, size - 1, 0, size - 1);
		queue.enqueueWriteBuffer(buffer_quad, CL_FALSE, 0, sizeof(Quad), &quad_inits.back());
		
		for (int j = 0; j < logN; ++j)
		{
			queue.enqueueNDRangeKernel(step_1_2,
				cl::NullRange,
				cl::NDRange(1 << j),
				cl::NullRange);
		
		}
		queue.enqueueNDRangeKernel(step_2,
			cl::NullRange,
			cl::NDRange(num_compute_unit),
			cl::NullRange);
	}
	///////////////////////////////////////
	// Generate B array
	///////////////////////////////////////
	//     step1point2
	cl::Kernel step_1_2_rev(program, "Step1Point2_reverse", &err);
	printIfError(err, "Step1Point2_reverse kernel error");
	step_1_2_rev.setArg(0, buffer_quad);
	step_1_2_rev.setArg(1, buffer_tmp);
	step_1_2_rev.setArg(2, buffer_tmp2);
	//     step2
	cl::Kernel step_2_rev(program, "Step2_reverse", &err);
	printIfError(err, "Step2 kernel error");
	step_2_rev.setArg(0, buffer_tmp);
	step_2_rev.setArg(1, buffer_tmp2);
	step_2_rev.setArg(2, buffer_B);
	step_2_rev.setArg(3, buffer_quad);
	for (int i = 0; i < BSize; ++i)
	{
		const cl_int size = (1ll << i);
		const cl_int bytesize = sizeof(Triple) * size;
		queue.enqueueCopyBuffer(buffer_B, buffer_tmp, 0, 0, bytesize);
		queue.enqueueWriteBuffer(buffer_arg, CL_FALSE, 0, sizeof(Triple), &(B.at(i)));
		add_triple.setArg(2, buffer_arg);
		queue.enqueueNDRangeKernel(add_triple,
			cl::NullRange,
			cl::NDRange(size),
			cl::NullRange);
		quad_inits.emplace_back(0, size - 1, 0, size - 1);
		queue.enqueueWriteBuffer(buffer_quad, CL_FALSE, 0, sizeof(Quad), &quad_inits.back());
		for (int j = 0; j < logN; ++j)
		{
			queue.enqueueNDRangeKernel(step_1_2_rev,	
				cl::NullRange,
				cl::NDRange(1 << j),
				cl::NullRange);
		}
		queue.enqueueNDRangeKernel(step_2_rev,
			cl::NullRange,
			cl::NDRange(num_compute_unit),
			cl::NullRange);
	}
	/*printBuffer<cl_int>(1 << 5, buffer_A, "buffer_A");
	printBuffer<cl_int>(1 << 5, buffer_B, "buffer_B");*/
}

void IntelCPUParallelCopa::parallel_generation()
{
	cl_int err;
	// buffer init
	cl::Buffer buffer_tmp = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(Triple) * (1ll << ((ASize > BSize ? ASize : BSize) - 1)), NULL, &err);
	printIfError(err, "buffer_tmp");
	cl::Buffer buffer_tmp2 = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(Triple) * (1ll << ((ASize > BSize ? ASize : BSize) - 1)), NULL, &err);
	printIfError(err, "buffer_tmp2");
	cl::Buffer buffer_arg = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(Triple), NULL, &err);
	printIfError(err, "buffer_arg");
	// initial values
	const Triple init{ 1, 0, 0 };
	err = queue.enqueueWriteBuffer(buffer_A, CL_FALSE, 0, sizeof(Triple), &init);
	printIfError(err, "enqueue initial value to buffer_A");
	err = queue.enqueueWriteBuffer(buffer_B, CL_FALSE, 0, sizeof(Triple), &init);
	printIfError(err, "enqueue initial value to buffer_B");
	// define common kernels
	cl::Kernel add_triple(program, "add_triple", &err);
	printIfError(err, "add_triple kernel error");
	add_triple.setArg(0, buffer_tmp);
	add_triple.setArg(1, buffer_tmp2);
	add_triple.setArg(2, buffer_arg);
	// set sequential threshold
	int seq_threshold = 0;
	cl_int _num_compute_unit = num_compute_unit;
	while (_num_compute_unit) {
		_num_compute_unit >>= 1;
		++seq_threshold;
	}

	// ----------------------------
	// Parallel Generation for A
	// ----------------------------
	cl::Kernel merge_array_sequential_a(program, "merge_array_sequential_a", &err);
	printIfError(err, "merge_array_sequential_a kernel error");
	merge_array_sequential_a.setArg(0, buffer_A);
	merge_array_sequential_a.setArg(1, buffer_tmp);
	merge_array_sequential_a.setArg(2, buffer_tmp2);
	cl::Kernel merge_array_parallel_a(program, "merge_array_parallel_a");
	merge_array_parallel_a.setArg(0, buffer_A);
	merge_array_parallel_a.setArg(1, buffer_tmp);
	merge_array_parallel_a.setArg(2, buffer_tmp2);
	for (size_t i = 0, seq_size = (ASize < seq_threshold ? ASize : seq_threshold); i < seq_size; ++i)
	{
		// sequnetial merge
		const cl_int size = (1ll << i);
		const cl_int bytesize = sizeof(Triple) * size;
		queue.enqueueCopyBuffer(buffer_A, buffer_tmp, 0, 0, bytesize);
		queue.enqueueWriteBuffer(buffer_arg, CL_FALSE, 0, sizeof(Triple), &(A.at(i)));
		queue.enqueueNDRangeKernel(add_triple,
			cl::NullRange,
			cl::NDRange(size),
			cl::NullRange);
		// err = queue.enqueueFillBuffer(buffer_A, -1, 0, bytesize << 1);
		printIfError(err, "error while filling buffer_A with -1");
		merge_array_parallel_a.setArg(3, sizeof(cl_int), &size);
		queue.enqueueNDRangeKernel(merge_array_sequential_a,
			cl::NullRange,
			cl::NDRange(1),
			cl::NDRange(1));
	}
	for (int i = seq_threshold; i < ASize; ++i)
	{
		const cl_int size = (1ll << i);
		const cl_int bytesize = sizeof(Triple) * size;
		queue.enqueueCopyBuffer(buffer_A, buffer_tmp, 0, 0, bytesize);
		queue.enqueueWriteBuffer(buffer_arg, CL_FALSE, 0, sizeof(Triple), &(A.at(i)));
		add_triple.setArg(2, buffer_arg);
		queue.enqueueNDRangeKernel(add_triple,
			cl::NullRange,
			cl::NDRange(size),
			cl::NullRange);
		for (_num_compute_unit = num_compute_unit; _num_compute_unit > 0; _num_compute_unit >>= 1) {
			cl_int local_size = size / _num_compute_unit;
			merge_array_parallel_a.setArg(3, sizeof(cl_int), &local_size);
			queue.enqueueNDRangeKernel(merge_array_parallel_a,
				cl::NullRange,
				cl::NDRange(_num_compute_unit),
				cl::NDRange(1));
		}
	}
	// ----------------------------
	// Parallel Generation for B
	// ----------------------------
	cl::Kernel merge_array_sequential_b(program, "merge_array_sequential_b", &err);
	printIfError(err, "merge_array_sequential_b kernel error");
	merge_array_sequential_b.setArg(0, buffer_B);
	merge_array_sequential_b.setArg(1, buffer_tmp);
	merge_array_sequential_b.setArg(2, buffer_tmp2);
	cl::Kernel merge_array_parallel_b(program, "merge_array_parallel_b");
	merge_array_parallel_b.setArg(0, buffer_B);
	merge_array_parallel_b.setArg(1, buffer_tmp);
	merge_array_parallel_b.setArg(2, buffer_tmp2);
	for (size_t i = 0, seq_size = (BSize < seq_threshold ? BSize : seq_threshold); i < seq_size; ++i)
	{
		// sequnetial merge
		const cl_int size = (1ll << i);
		const cl_int bytesize = sizeof(Triple) * size;
		queue.enqueueCopyBuffer(buffer_B, buffer_tmp, 0, 0, bytesize);
		queue.enqueueWriteBuffer(buffer_arg, CL_FALSE, 0, sizeof(Triple), &(B.at(i)));
		queue.enqueueNDRangeKernel(add_triple,
			cl::NullRange,
			cl::NDRange(size),
			cl::NullRange);
		merge_array_sequential_b.setArg(3, sizeof(cl_int), &size);
		queue.enqueueNDRangeKernel(merge_array_sequential_b,
			cl::NullRange,
			cl::NDRange(1),
			cl::NDRange(1));
	}
	for (int i = seq_threshold; i < BSize; ++i)
	{
		const cl_int size = (1ll << i);
		const cl_int bytesize = sizeof(Triple) * size;
		queue.enqueueCopyBuffer(buffer_B, buffer_tmp, 0, 0, bytesize);
		queue.enqueueWriteBuffer(buffer_arg, CL_FALSE, 0, sizeof(Triple), &(B.at(i)));
		add_triple.setArg(2, buffer_arg);
		queue.enqueueNDRangeKernel(add_triple,
			cl::NullRange,
			cl::NDRange(size),
			cl::NullRange);
		for (_num_compute_unit = num_compute_unit; _num_compute_unit > 0; _num_compute_unit >>= 1) {
			cl_int local_size = size / _num_compute_unit;
			merge_array_parallel_b.setArg(3, sizeof(cl_int), &local_size);
			queue.enqueueNDRangeKernel(merge_array_parallel_b,
				cl::NullRange,
				cl::NDRange(_num_compute_unit),
				cl::NDRange(1));
		}
		// devel only -----------------------------------------------------------
		//printBuffer(size, bytesize);
		// devel only -----------------------------------------------------------
	}
	//printBuffer(A)
	/*printBuffer<cl_int>(1<<5, buffer_A, "buffer_A");
	printBuffer<cl_int>(1<<5, buffer_B, "buffer_B");*/
}

const cl_int MINUS_ONE = -1;
const cl_int ZERO = 0;

void IntelCPUParallelCopa::first_max_scan()
{
	cl_int err;
	const size_t bytesize = sizeof(AlignedInt) * num_compute_unit;
	const cl_int ABufferSize = (1 << ASize);
	buffer_AMaxValues = cl::Buffer(context, CL_MEM_READ_WRITE, bytesize);
	buffer_AMaxValuesIdx = cl::Buffer(context, CL_MEM_READ_WRITE, bytesize);
	cl::Kernel get_max_value_a(program, "first_max_scan", &err);
	printIfError(err, "get_max_value_a kernel error");
	get_max_value_a.setArg(0, buffer_A);
	get_max_value_a.setArg(1, buffer_AMaxValues);
	get_max_value_a.setArg(2, buffer_AMaxValuesIdx);
	get_max_value_a.setArg(3, sizeof(cl_int), &MINUS_ONE);
	//queue.enqueueFillBuffer(buffer_AMaxValues, -1, 0, bytesize);
	queue.enqueueNDRangeKernel(get_max_value_a,
		cl::NullRange,
		cl::NDRange(ABufferSize),
		num_work_item);

	const cl_int BBufferSize = (1 << BSize);
	buffer_BMaxValues = cl::Buffer(context, CL_MEM_READ_WRITE, bytesize);
	buffer_BMaxValuesIdx = cl::Buffer(context, CL_MEM_READ_WRITE, bytesize);
	cl::Kernel get_max_value_b(program, "first_max_scan", &err);
	printIfError(err, "get_max_value_b kernel error");
	get_max_value_b.setArg(0, buffer_B);
	get_max_value_b.setArg(1, buffer_BMaxValues);
	get_max_value_b.setArg(2, buffer_BMaxValuesIdx);
	get_max_value_b.setArg(3, sizeof(cl_int), &MINUS_ONE);
	//queue.enqueueFillBuffer(buffer_BMaxValues, -1, 0, bytesize);
	queue.enqueueNDRangeKernel(get_max_value_b,
		cl::NullRange,
		cl::NDRange(BBufferSize),
		num_work_item);
	/*printBuffer<cl_int>(num_compute_unit, buffer_AMaxValues, "AMaxValues: after first scan ");
	printBuffer<cl_int>(num_compute_unit, buffer_BMaxValues, "BMaxValues: after first scan ");*/
}


void IntelCPUParallelCopa::prune()
{
	const size_t buffers_pruned_bytesize = sizeof(Pair) * num_compute_unit * 2;
	//(1ll << ASize)
	buffer_pruned = cl::Buffer(context, CL_MEM_READ_WRITE, buffers_pruned_bytesize);
	queue.enqueueFillBuffer(buffer_pruned, -1, 0, buffers_pruned_bytesize);
	buffer_first_max_val = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(AlignedInt)*num_compute_unit);
	buffer_first_max_val_pair = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(AlignedInt) * num_compute_unit);
	cl_int err;
	cl::Kernel prune_kernel(program, "prune", &err);
	printIfError(err, "error in prune_parallel ");

	const cl_int c = data.getCapacity();
	prune_kernel.setArg(0, buffer_A);
	prune_kernel.setArg(1, buffer_B);
	prune_kernel.setArg(2, buffer_pruned);
	prune_kernel.setArg(3, buffer_AMaxValues);
	prune_kernel.setArg(4, buffer_BMaxValues);
	prune_kernel.setArg(5, buffer_AMaxValuesIdx);
	prune_kernel.setArg(6, buffer_BMaxValuesIdx);
	prune_kernel.setArg(7, buffer_first_max_val);
	prune_kernel.setArg(8, buffer_first_max_val_pair);
	prune_kernel.setArg(9, sizeof(cl_int), &A_local_size);
	prune_kernel.setArg(10, sizeof(cl_int), &B_local_size);
	prune_kernel.setArg(11, sizeof(cl_int), &c);
	prune_kernel.setArg(12, sizeof(cl_int), &num_compute_unit);
	prune_kernel.setArg(13, sizeof(cl_int), &MINUS_ONE);
	prune_kernel.setArg(14, sizeof(cl_int), &ZERO);
	queue.enqueueNDRangeKernel(prune_kernel,
		cl::NullRange,
		cl::NDRange(num_compute_unit*num_compute_unit),
		cl::NDRange(num_compute_unit));
	/*printBuffer<Pair>(num_compute_unit * 2, buffer_pruned,"after prunning");*/
}

void IntelCPUParallelCopa::second_max_scan()
{
	const size_t buffer_max_size = sizeof(cl_int) * 2 * B_local_size * num_compute_unit;
	cl_int err;
	buffer_max = cl::Buffer(context, CL_MEM_READ_WRITE, buffer_max_size, NULL, &err);
	queue.enqueueFillBuffer(buffer_max, -1, 0, buffer_max_size);
	printIfError(err, "buffer_max");
	cl::Kernel second_max_scan_kernel(program, "second_max_scan", &err);
	printIfError(err, "error in second_max_scan");
	second_max_scan_kernel.setArg(0, buffer_A);
	second_max_scan_kernel.setArg(1, buffer_B);
	second_max_scan_kernel.setArg(2, buffer_pruned);
	second_max_scan_kernel.setArg(3, buffer_max);
	second_max_scan_kernel.setArg(4, sizeof(cl_int), &A_local_size);
	second_max_scan_kernel.setArg(5, sizeof(cl_int), &B_local_size);
	// printBuffer<Pair>(2 * num_compute_unit, buffer_pruned, "second_max_scan");
	queue.enqueueNDRangeKernel(second_max_scan_kernel,
		cl::NullRange,
		cl::NDRange(num_compute_unit),
		cl::NDRange(1));
	
}

void IntelCPUParallelCopa::final_search()
{
	const size_t buffer_max_bytesize = num_compute_unit * sizeof(cl_int);
	const size_t buffer_max_pair_bytesize = num_compute_unit * sizeof(Pair);
	cl_int err;
	buffer_second_max_val = cl::Buffer(context, CL_MEM_READ_WRITE, buffer_max_bytesize, NULL, &err);
	printIfError(err, "buffer_max_val");
	buffer_second_max_val_pair = cl::Buffer(context, CL_MEM_READ_WRITE, buffer_max_pair_bytesize, NULL, &err);
	printIfError(err, "buffer_max_val_pair");
	const cl_int c = data.getCapacity();
	cl::Kernel final_search_kernel(program, "final_search", &err);
	printIfError(err, "error in final_search");
	final_search_kernel.setArg(0, buffer_A);
	final_search_kernel.setArg(1, buffer_B);
	final_search_kernel.setArg(2, buffer_pruned);
	final_search_kernel.setArg(3, buffer_max);
	final_search_kernel.setArg(4, buffer_second_max_val);
	final_search_kernel.setArg(5, buffer_second_max_val_pair);
	final_search_kernel.setArg(6, sizeof(cl_int), &A_local_size);
	final_search_kernel.setArg(7, sizeof(cl_int), &B_local_size);
	final_search_kernel.setArg(8, sizeof(cl_int), &c);
	final_search_kernel.setArg(9, sizeof(cl_int), &num_compute_unit);
	queue.enqueueNDRangeKernel(final_search_kernel,
		cl::NullRange,
		cl::NDRange(num_compute_unit),
		cl::NDRange(1));

	// find solution in first scan
	std::vector<cl_int> max_val(num_compute_unit);
	std::vector<cl_int> max_val_set(num_compute_unit);
	queue.enqueueReadBuffer(buffer_first_max_val, CL_FALSE, 0, buffer_max_bytesize, max_val.data());
	queue.enqueueReadBuffer(buffer_first_max_val_pair, CL_TRUE, 0, buffer_max_bytesize, max_val_set.data());
	int maxi = 0; solution_value = max_val.at(0);
	for (int i = 1; i < max_val.size(); ++i)
	{
		if (max_val.at(i) > solution_value) {
			solution_value = max_val.at(i);
			maxi = i;
		}
	}
	solution_set = std::bitset<64>(max_val_set.at(maxi));

	// find solution in second scan
	std::vector<Pair> max_val_pair(num_compute_unit);
	queue.enqueueReadBuffer(buffer_second_max_val, CL_FALSE, 0, buffer_max_bytesize, max_val.data());
	queue.enqueueReadBuffer(buffer_second_max_val_pair, CL_TRUE, 0, buffer_max_pair_bytesize, max_val_pair.data());
	maxi = -1;
	for (int i = 0; i < max_val.size(); ++i)
	{
		if (max_val.at(i) >= solution_value)
		{
			solution_value = max_val.at(i);
			maxi = i;
		}
	}
	if (maxi >= 0) {
		solution_set = std::bitset<64>(max_val_pair.at(maxi).a_idx + max_val_pair.at(maxi).b_idx);
	}
}