#pragma once

#include <CL/cl.hpp>

#if defined(_WIN32) || defined(WIN32)
#define PACKED __declspec(align(8))
#elif defined(__APPLE__) || defined(__MACH__) || defined(__linux__)
#define PACKED __attribute__((aligned(8)))
#else 
#error Unknown OS
#endif

struct PACKED Triple
{
	Triple() : set(0), w(0), p(0) { }
	Triple(cl_int set, cl_int w, cl_int p) : set(set), w(w), p(p) { }
	cl_long set;				//Set denotes which Elements are in the Subset
	cl_int w;					//Weight of the Triple
	cl_int p;					//Profit of the Triple
};

