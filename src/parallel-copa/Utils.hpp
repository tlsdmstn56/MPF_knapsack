#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <utility>
#include <stdexcept>

#include <CL/cl.hpp>

std::string GetSourceFromFile(const std::string& filename);
int GetInput(const std::string& prompt, const int min, const int max);

class CLFileNotExistException : public std::runtime_error
{
public:
	CLFileNotExistException(const std::string& filepath) : std::runtime_error(filepath)
	{
		this->filepath = filepath;
		errmsg = filepath + " is not exists";
	}
	CLFileNotExistException(std::string&& filepath) : std::runtime_error(filepath)
	{
		std::swap(this->filepath, filepath);
		errmsg = filepath + " is not exists";
	}
	const char* what() const
	{
		return errmsg.c_str();
	}
private:
	std::string filepath;
	std::string errmsg;
};