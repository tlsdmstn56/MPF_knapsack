find_package(OpenCL REQUIRED)
include_directories(${OpenCL_INCLUDE_DIRS})
link_directories(${OpenCL_LIBRARY})

add_executable(
    parallel_copa
    parallel_copa.cpp
	Utils.cpp
	IntelCPUParallelCopa.cpp
)
target_link_libraries (parallel_copa 
    ${OpenCL_LIBRARY}
    ${PAPI_LIBRARY}
)

# OS specific
if (WIN32)
	add_custom_command(
    TARGET parallel_copa PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/copa_kernels.cl
        ${CMAKE_SOURCE_DIR})
elseif(UNIX)
    add_custom_command(
    TARGET parallel_copa PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/copa_kernels.cl
        ${CMAKE_BINARY_DIR})
endif()

