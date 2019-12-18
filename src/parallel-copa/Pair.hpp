#pragma once

#include <CL/cl.hpp>

#if defined(_WIN32) || defined(WIN32)
#define PACKED __declspec(align(8))
#elif defined(__APPLE__) || defined(__MACH__) || defined(__linux__)
#define PACKED __attribute__((aligned(8)))
#else 
#error Unknown OS
#endif



struct PACKED Pair
{
	Pair() :a_idx(0), b_idx(0) {}
	cl_int a_idx;
	cl_int b_idx;
};
