#pragma once

#include <random>
#include <utility>
#include <common/Data.hpp>

class DataGenerator {
public:
    static Data generate(const int32_t size, const int32_t seed) noexcept {
        std::mt19937 generator(seed);
        std::uniform_int_distribution<int64_t> distr(1, 10000000);
        Data ret;
        int64_t c = 0, wt, p;
        for (int32_t i = 0; i < size; ++i)
        {
            
            wt = distr(generator);
            p = distr(generator);
            c += wt;
            ret.addEntry(std::move(std::make_pair(wt, p)));
        }
        c /= 2;
        return ret;
  }
};