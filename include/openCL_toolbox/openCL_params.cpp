#ifndef OPENCL_PARAMS_CPP
#define OPENCL_PARAMS_CPP
#include "openCL_params.h"

////
// implementation
////
namespace openCL_toolbox
{
	openCL_params::openCL_params(const int chunkSize_) : chunkSize(chunkSize_)
	{
		accelerators = std::vector<cl::Device>({cl::Device::getDefault()});
	}

	openCL_params::~openCL_params()
	{
	}
}

#endif
