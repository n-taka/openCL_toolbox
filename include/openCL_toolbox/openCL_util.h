#ifndef OPENCL_UTIL_H
#define OPENCL_UTIL_H
#define __CL_ENABLE_EXCEPTIONS

#include "openCL_params.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "cl.hpp"

////
// implementation
////

namespace openCL_toolbox
{

void getAcceleratorDescription(const std::vector<cl::Device> &accelerators);
void getAcceleratorDescription(const std::unordered_map<std::string, std::vector<std::pair<int, cl::Device>>> &platform_devices, const int uniqueIdx = -1);

void getAccelerator(std::unordered_map<std::string, std::vector<std::pair<int, cl::Device>>> &platform_devices);

void selectAccelerator(const std::unordered_map<std::string, std::vector<std::pair<int, cl::Device>>> &platform_devices, const std::unordered_set<int> &acceleratorIndices, openCL_toolbox::openCL_params& params);
void selectAccelerator(openCL_params& params);

}

#include "openCL_util.cpp"

#endif
