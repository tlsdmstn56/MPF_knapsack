#include <iostream>
#include <cassert>
#include <string>

#include "Utils.hpp"


std::string GetSourceFromFile(const std::string& filename) {
	std::ifstream t(filename);
	if (!t.good())
	{
		throw CLFileNotExistException(filename);
	}
	return { std::istreambuf_iterator<char>(t), std::istreambuf_iterator<char>() };
}

int GetInput(const std::string& prompt, const int min, const int max) {
	int i = min - 1;
	std::cout << prompt << " ";
	std::cin >> i;
	while (i<min || i>max) {
		std::cout << "invalid input: must be between " << min << " and " << max << "\n";
		std::cout << prompt << " ";
		std::cin >> i;
	}
	return i;
}