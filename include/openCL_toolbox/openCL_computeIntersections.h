#ifndef OPENCL_COMPUTE_INTERSECTIONS
#define OPENCL_COMPUTE_INTERSECTIONS
#define __CL_ENABLE_EXCEPTIONS

#include "Eigen/Core"
#include "cl.hpp"
#include "igl/Hit.h"

void openCL_computeIntersections(
	const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> &V,
	const Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic> &F,
	const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> &raySources,
	const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> &dirs,
	const int &chunkSize,
	const cl::Device &device,
	std::vector<std::vector<igl::Hit>> &hits);

#endif
