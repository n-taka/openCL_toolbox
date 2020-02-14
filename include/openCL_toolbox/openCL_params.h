#ifndef OPENCL_PARAMS_H
#define OPENCL_PARAMS_H
#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120

#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
#include <CL/cl2.hpp>
#elif defined(__APPLE__)
// cl2.hpp is not installed within macOS
// https://github.com/KhronosGroup/OpenCL-CLHPP/releases
#include "OpenCL/cl2.hpp"
#endif
#include <vector>

////
// implementation
////

namespace openCL_toolbox
{

class openCL_params
{
public:
    inline openCL_params(const int chunkSize_ = 100000);
    inline ~openCL_params();

    std::vector<cl::Device> accelerators;
    int chunkSize;
};

} // namespace openCL_toolbox

#include "openCL_params.cpp"

#endif
