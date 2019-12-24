#pragma once

#include <CL/cl.hpp>

#if defined(_WIN32) || defined(WIN32)
#define CACHELINE_ALIGNED __declspec(align(64))
#elif defined(__APPLE__) || defined(__MACH__) || defined(__linux__)
#define CACHELINE_ALIGNED __attribute__((aligned(64)))
#else 
#error Unknown OS
#endif

struct CACHELINE_ALIGNED Quad
{
	Quad() : startA(0), endA(0), startB(0), endB(0) {}
	Quad(cl_int startA, cl_int endA, cl_int startB, cl_int endB): 
		startA(startA), endA(endA), startB(startB), endB(endB) {}
	cl_int startA, endA, startB, endB;
};