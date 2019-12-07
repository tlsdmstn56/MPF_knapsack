#pragma once

#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#include <exception>
class InitializerError : public std::exception
{
    const char *what() const throw()
    {
        return "QueryPerformanceFrequency is not supported";
    }
};

class Timer
{
private:
    int64_t startFreq=-1;
    LARGE_INTEGER li;
    double freq_to_ns;
public:
	Timer() {
        if (!QueryPerformanceFrequency(&li))
        {
            throw InitializerError();
        };
        freq_to_ns = 1.0e9 / static_cast<double>(li.QuadPart);
    };
    void start() noexcept
    {
        QueryPerformanceCounter(&li);
        startFreq = li.QuadPart;
    }
    int64_t stop() noexcept
    {
        QueryPerformanceCounter(&li);
        int64_t endFreq = li.QuadPart;
        return static_cast<int64_t>((endFreq - startFreq) * freq_to_ns);
    }
};

#elif defined(__APPLE__) || defined(__MACH__) || defined(__linux__)

extern "C"
{
#include <papi.h>
}

#include <exception>
class InitializerError : public std::exception
{
    const char *what() const throw()
    {
        return "PAPI Library initialization error";
    }
};

class Timer
{
private:
    int64_t startNS;
public:
    Timer() {
        if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT)
        {
            throw InitializerError();
        }
    }
    void start() noexcept {
        startNS = PAPI_get_virt_nsec();
    } 
    int64_t stop() const noexcept 
    {
        return PAPI_get_virt_nsec() - startNS;
    }
};

#else
#error Unknown OS
#endif
