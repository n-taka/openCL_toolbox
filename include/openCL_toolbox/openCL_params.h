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
    inline openCL_params(const int chunkSize_ = 100000);
    inline ~openCL_params();

    std::vector<cl::Device> accelerators;
    int chunkSize;
};

} // namespace openCL_toolbox

#include "openCL_params.cpp"

#endif
