#ifndef OPENCL_PARAMS_H
#define OPENCL_PARAMS_H
#define __CL_ENABLE_EXCEPTIONS

#include "cl.hpp"
#include <vector>

////
// implementation
////

namespace openCL_toolbox
{

class openCL_params
{
public:
    openCL_params(const int chunkSize_ = 100000);
    ~openCL_params();
    
    std::vector<cl::Device> accelerators;
    int chunkSize;
};

}

#include "openCL_params.cpp"

#endif
