# MPF knapsack

## Requirement

* cmake(>3.5)

## Clone submodules
```bash
# in project root directory(macOS/linux)
git submodule update --init --recursive
cd external/papi
./configure
make
make test # for test
```

```bash
# windows
# windows only supports 3.7.1
git submodule update --init --recursive
cd external/papi
git checkout tags/papi-3-7-1
```

## build

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
