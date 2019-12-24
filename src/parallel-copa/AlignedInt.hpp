#pragma once

#include <CL/cl.hpp>

#if defined(_WIN32) || defined(WIN32)
#define CACHELINE_ALIGNED __declspec(align(64))
#elif defined(__APPLE__) || defined(__MACH__) || defined(__linux__)
#define CACHELINE_ALIGNED __attribute__((aligned(64)))
#else 
#error Unknown OS
#endif

struct CACHELINE_ALIGNED AlignedInt {
	AlignedInt(cl_int val): val(val) { }
	AlignedInt() : val(0) { }
	cl_int val;
};