cmake_minimum_required(VERSION 3.5)

project (mpf_knapsack)

# OS specific
if (WIN32)

elseif(UNIX)
    # compiler setup
    set(CMAKE_CXX_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
    # compile flags
    SET (CMAKE_C_FLAGS_INIT                "-Wall -Wextra")
    SET (CMAKE_C_FLAGS_DEBUG_INIT          "-g")
    SET (CMAKE_C_FLAGS_MINSIZEREL_INIT     "-Os -DNDEBUG")
    SET (CMAKE_C_FLAGS_RELEASE_INIT        "-O3 -DNDEBUG")
    SET (CMAKE_C_FLAGS_RELWITHDEBINFO_INIT "-O2 -g")
    SET (CMAKE_CXX_FLAGS_INIT                "-Wall -Wextra")
    SET (CMAKE_CXX_FLAGS_DEBUG_INIT          "-g")
    SET (CMAKE_CXX_FLAGS_MINSIZEREL_INIT     "-Os -DNDEBUG")
    SET (CMAKE_CXX_FLAGS_RELEASE_INIT        "-O3 -DNDEBUG")
    SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "-O2 -g")
endif()


# build standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


# include path
if (WIN32)
    include_directories(
        ./src
		./external/cxxopts/include
    )
    set(PAPI_LIBRARY "")
elseif(UNIX)
    include_directories(
        ./src
        ./external/papi/src
		./external/cxxopts/include
    )
    link_directories(
        ./external/papi/src
    )
    set(PAPI_LIBRARY "papi")
endif()



# include directories
add_subdirectory(src) 