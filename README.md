# MPF knapsack

## Requirement

* cmake(>3.5)

## `papi` 설치

mac/ linux 사용자만

```bash
# in project root directory(macOS/linux)
git submodule update --init --recursive
cd external/papi
./configure
make
```

`libpfm` 설치해야 될 수도 있음

## 빌드

```bash
# mac/ linux
mkdir build
cd build
cmake ..
make
```

```powershell
# powershell(windows)
mkdir build
cd build
cmake ..

cmake --build .
# or
msbuild mpf_knapsack.sln
```

## 실행

바이너리는 `build/bin` 에 저장되어 있음

## `common` 사용 방법

### `Timer`

내부적으로 `papi`를 사용, 윈도우는 지원하지 않아서 윈도우 커널 함수인 `QueryPerformanceCounter`를 사용

* `papi`는 프로세스가 실제로 실행된 시간만큼만 게산(sleep되거나 context 스위치 된 시간은 제외)
* `QueryPerformanceCounter`: 단순히 현재 타임스탬프 값을 계산

```cpp
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

### `Data` and `DataGenerator`

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