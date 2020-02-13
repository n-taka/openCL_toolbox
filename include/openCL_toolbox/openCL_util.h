#ifndef OPENCL_UTIL_H
#define OPENCL_UTIL_H
#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 120

#include "openCL_params.h"

#include <vector>
#include <string>
#include <unordered_set>
#include <CL/cl2.hpp>

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
