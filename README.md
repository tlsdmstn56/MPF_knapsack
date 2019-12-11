# MPF knapsack

## Requirement

* cmake(>3.5)
* `papi` 빌드

### `papi` 설치

mac/ linux 사용자만

```bash
# in project root directory(macOS/linux)
git submodule update --init --recursive
cd external/papi
./configure
make
```

`libpfm` 설치해야 될 수도 있음

## Build

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

* linux/ macOS: 바이너리는 `build/bin` 에 저장되어 있음
* Windows: 바이너리는 `build/bin/Debug`에

## `common` 

공통으로 사용되는 기능으로 `src/common`에 header-only 형태로 사용할 수 있음

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

`start`를 호출하면 타이머가 시작된다. `end`를 호출하면 현재까지 지난 시간을 반환한다.

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

`DataGenerator::generate`을 실행하면 된다. 첫 파라미터는 데이터의 수, 두 번째 파라미터는 seed 값이다. 시드값이 같을 때 항상 같은 랜덤 데이터를 리턴한다.

## Parallel knapsack

다양한 환경에서 테스트를 할 수 있도록 cli 인터페이스가 구현되어 있다.

주의할 점으로는 실행하는 위치에 `copa_kernels.cl`파일이 있어야 한다. 파일이 없다면 복사를 하거나 `-f` 옵션으로 위치를 지정할 수 있다.

```bash
# default 값으로 실행
./parallel_copa

# 파일 위치 지정
./parallel_copa -f ../../../copa_kernels.cl
```

실행 결과 예시

```
Selected Platform: Intel(R) OpenCL
Selected Device:   Intel(R) Core(TM) i7-8550U CPU @ 1.80GHz
Solution:     40927966
Solution Set: 1000111101
Elapsed Time: 841341800ns
```

### 플랫폼, 디바이스 바꾸기

Intel 외에 다른 플랫폼에서 실행하고자 하는 경우 실행가능한 플랫폼과 디바이스를 확인한다.

```bash
# 버전이 다른 Opencl 2개가 설치되어 있어 2개가 나옴 NVidia나 AMD GPU도 같은 방법으로 가능(테스트 필요)
./parallel_copa --list
Platforms
------------------
[0] Intel(R) OpenCL
  - [0] Intel(R) UHD Graphics 620
  - [1] Intel(R) Core(TM) i7-8550U CPU @ 1.80GHz
[1] Intel(R) OpenCL
  - [0] Intel(R) UHD Graphics 620
  - [1] Intel(R) Core(TM) i7-8550U CPU @ 1.80GHz
```

인덱스 번호를 확인하고 파라미터로 넘기면 된다.

```
./parallel_copa -p 0 -d 1
```

### 스레드 수 바꾸기

`-j` 옵션을 사용한다.

### 기타 옵션

`--help`로 사용 가능한 옵션을 확인할 수 있다.

```powershell
# powershell
PS C:\MPF_knapsack\build\bin\Debug> .\parallel_copa.exe --help
Solve kanpsack problem on heterogeous cores using OpenCL
Usage:
  C:\MPF_knapsack\build\bin\Debug\parallel_copa.exe [OPTION...]

  -d, --device arg        Device Index (default: 0)
  -p, --platform arg      Platform Index (default: 0)
  -n, arg                 The number of data (default: 10)
  -j, arg                 the number of threads (default: 8)
  -s, --seed arg          Seed for data generator (default: 0)
  -f, --cl_file_path arg  path of copa_kernels.cl file (default:
                          copa_kernels.cl)
      --list              List platforms and devices
  -h, --help              Print help
```

