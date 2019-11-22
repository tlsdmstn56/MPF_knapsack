find_package(OpenCL REQUIRED)
include_directories(${OpenCL_INCLUDE_DIRS})
link_directories(${OpenCL_LIBRARY})

add_executable(
    parallel_copa
    parallel_copa.cpp
)
target_link_libraries (parallel_copa 
    ${OpenCL_LIBRARY}
    ${PAPI_LIBRARY}
)