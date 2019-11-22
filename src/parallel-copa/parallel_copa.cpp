#include <iostream>
#include <thread>
#include <chrono>

#include <CL/cl.h>

#include <common/Data.hpp> // Data
#include <common/DataGenerator.hpp> // DataGenerator
#include <common/Timer.hpp> // Timer

#include "CLKernels.hpp"

int main() {
    using namespace std::chrono_literals;
    
    Timer timer;
    timer.start();
    Data data = DataGenerator::generate(10, 43);
    std::cout << "(weight, price)\n";
    for(const auto& e: data.getTable()) {
        std::cout << "(" << e.first << ", " << e.second << ")\n";
    }
    std::this_thread::sleep_for(2s);
    std::cout << timer.stop() << "\n";
    return 0;
}