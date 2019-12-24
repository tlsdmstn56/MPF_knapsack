# MPF knapsack

## Requirement

* cmake (3.5+)
* `papi`

### Initializing submodules

```bash
# in project root directory
git submodule update --init --recursive
```

### Building `papi` 

`papi` works only on Linux. This part is not necessary in Windows or MacOS.

```bash
# in project root directory(Linux)
git submodule update --init --recursive
cd external/papi
./configure
make
```

> Note: `libpfm`  might be necessary to build `papi`

## Build

### Debug build

```bash
# mac/ Linux
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

```powershell
# powershell(windows)
mkdir build
cd build
cmake ..

cmake --build . --config Debug
# or
msbuild mpf_knapsack.sln
```

### Release build
```bash
# mac/ Linux
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

```powershell
# powershell(windows)
mkdir build
cd build
cmake ..

cmake --build . --config Release
# or
msbuild mpf_knapsack.sln
```


## Run

* Linux/ macOS: run one of binaries in `build/bin` 
* Windows: run one of binaries in `build/bin/Debug` or `build/bin/Release`

>  Warning: to run `parallel_copa`, the location of `copa_kernels.cl` must be specified

```bash
# default location is ./src/parallel_copa/copa_kernels.cl
./parallel_copa

# specifying the location of copa_kernels.cl 
./parallel_copa -f /home/test/copa_kernels.cl
```

### CLI interface

You can configure the experiment by passing parameter as command line argument.

#### Query available platforms and devices

Only for `parallel_copa`

```bash
PS C:\Users\test\MPF_knapsack\build\bin\Release> .\parallel_copa.exe --list
------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
|       Platform       |                    Device                     | Max CU size | Max WG Size | Max Globel Mem | Global Mem Cache Size | Global Mem Cacheline Size | Local Mem Size |
------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
| [0] Intel(R) OpenCL  | [0] Intel(R) UHD Graphics 620                 |          24 |         256 |     6811119616 |                524288 |                        64 |          65536 |
| [0] Intel(R) OpenCL  | [1] Intel(R) Core(TM) i7-8550U CPU @ 1.80GHz  |           8 |        8192 |    17027801088 |                262144 |                        64 |          32768 |
| [1] Intel(R) OpenCL  | [0] Intel(R) UHD Graphics 620                 |          24 |         256 |     6811119616 |                524288 |                        64 |          65536 |
| [1] Intel(R) OpenCL  | [1] Intel(R) Core(TM) i7-8550U CPU @ 1.80GHz  |           8 |        8192 |    17027801088 |                262144 |                        64 |          32768 |
------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
```

#### Available parameter

Use `--help`

```powershell
# powershell
PS C:\Users\test\MPF_knapsack\build\bin\Release> .\parallel_copa.exe --help
Solve kanpsack problem on heterogeous cores using OpenCL
Usage:
  C:\Users\test\MPF_knapsack\build\bin\Release\parallel_copa.exe [OPTION...]

  -p, --platform arg      Platform Index (default: 0)
  -d, --device arg        Device Index (default: 0)
  -c, --size_cu arg       the number of compute units to be used (default: 1)
  -w, --size_wg arg       the number of work items per a group (default: 1)
  -n, --data_size arg     The number of data (default: 10)
  -s, --seed arg          Seed for data generator (default: 0)
  -f, --cl_file_path arg  path of copa_kernels.cl file (default:
                          src/parallel-copa/copa_kernels.cl)
      --list              List platforms and devices
  -h, --help              Print help
```

## Development

### `common` module

Commonly used features (header-only in `src/common`)

#### `Timer`

It internally use `papi` (Linux). On Windows, it use `QueryPerformanceCounter` function in Window SDK.

> Warning: For MacOS, timer will be implemented in `std::chrono::high_resolution_clock`

There is a subtle difference between `papi` and `QueryPerformanceCounter`

* `papi` excludes the time the processed was slept or suspended
* `QueryPerformanceCounter`: includes the time the processed was slept or suspended

```cpp
/* Usage */
#include<iostream>
#include <thread>
#include <chrono>
#include <atomic>

#include <common/Timer.hpp> // Timer

std::atomic_int32_t v{0};

int main() {
    using namespace std::chrono_literals;
    
    Timer timer;
    timer.start();
    for(int i=0; i<100000000;++i) {
        v.fetch_add(1);
    }
    std::cout << timer.stop() << "\n";
    return 0;
}
```

By invoking `start`, timer keeps the start time and `end` function will return the elapsed time.

#### `Data` and `DataGenerator`

```cpp
#include <iostream>

#include <common/Data.hpp> // Data
#include <common/DataGenerator.hpp> // DataGenerator

int main() {
    Data data = DataGenerator::generate(10, 43);
    std::cout << "(weight, price)\n";
    for(const auto& e: data.getTable()) {
        // first: weight, second: value(or price)
        std::cout << "(" << e.first << ", " << e.second << ")\n";
    }
    return 0;
}
```

Just run `DataGenerator::generate`.

First parameter is the number of data and second parameter is a seed number for random number generator.