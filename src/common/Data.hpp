#pragma once

#include <vector>
#include <utility>

class Data {
public:
	Data() {}
	constexpr const std::vector<std::pair<int64_t, int64_t>>& getTable() const noexcept {
		return V;
	}
	constexpr int64_t getCapacity() const  noexcept {
		return capacity;
	}
	void setCapacity(int64_t capacity)  noexcept {
		this->capacity = capacity;
	}
	void addEntry(std::pair<int64_t, int64_t>&& entry) noexcept {
		V.push_back(std::move(entry));
	}
private:
	std::vector<std::pair<int64_t, int64_t>> V;
	int64_t capacity = -1;
};