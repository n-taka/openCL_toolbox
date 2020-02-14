#ifndef OPENCL_UTIL_H
#define OPENCL_UTIL_H
#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120

#include "openCL_params.h"

#include <vector>
#include <string>
#include <unordered_set>
#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
#include <CL/cl2.hpp>
#elif defined(__APPLE__)
// cl2.hpp is not installed within macOS
// https://github.com/KhronosGroup/OpenCL-CLHPP/releases
#include "OpenCL/cl2.hpp"
#endif

////
// implementation
////

namespace openCL_toolbox
{

inline void getAcceleratorDescription(const std::vector<cl::Device> &accelerators);
inline void getAcceleratorDescription(const std::vector<std::pair<cl::Platform, std::vector<cl::Device>>> &platforms_devices);

inline void getAccelerator(std::vector<std::pair<cl::Platform, std::vector<cl::Device>>> &platforms_devices);

inline void selectAccelerator(const std::vector<std::pair<cl::Platform, std::vector<cl::Device>>> &platforms_devices, const int &platformIndex, const std::unordered_set<int> &acceleratorIndices, openCL_toolbox::openCL_params &params);
inline void selectAccelerator(openCL_params &params);

} // namespace openCL_toolbox

#include "openCL_util.cpp"

#endif
