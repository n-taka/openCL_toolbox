#ifndef OPENCL_RAYMESHINTERSECTIONS_H
#define OPENCL_RAYMESHINTERSECTIONS_H
#define __CL_ENABLE_EXCEPTIONS

#include "Eigen/Core"
#pragma warning(push)
#pragma warning(disable : 4018 4129 4244 4267 4305 4566 4819 4996)
#include "igl/Hit.h"
#include "igl/AABB.h"
#pragma warning(pop)
#include "../openCL_params.h"

namespace openCL_toolbox
{

// Compute ray-mesh intersections
// Inputs:
//   V       #V   by 3 eigen Matrix of mesh vertex 3D positions
//   F       #F   by 3 eigen Matrix of face (triangle) indices
//   aabb    pre-computed axis-aligned bounding box
//           (for better performance)
//   RS      #Ray by 3 eigen Matrix of ray sources
//   D       #Ray by 3 eigen Matrix of ray directions
//   params  parameters for openCL
//   maxHit  how many hits we need
//           (if a ray hits more than maxHit-times, only nearest maxHit hits are stored)
// Output:
//   hits    #Ray vector of hits for each rays
//
template <
	typename DerivedV,
	typename DerivedF,
	typename DerivedRS,
	typename DerivedD>
void openCL_rayMeshIntersections(
	const Eigen::MatrixBase<DerivedV> &V,
	const Eigen::MatrixBase<DerivedF> &F,
	const Eigen::MatrixBase<DerivedRS> &RS,
	const Eigen::MatrixBase<DerivedD> &D,
	const openCL_toolbox::openCL_params &params,
	const int maxHit,
	std::vector<std::vector<igl::Hit>> &hits);

template <
	typename DerivedV,
	typename DerivedF,
	typename DerivedRS,
	typename DerivedD>
void openCL_rayMeshIntersections(
	const Eigen::MatrixBase<DerivedV> &V,
	const Eigen::MatrixBase<DerivedF> &F,
	const igl::AABB<DerivedV, 3> &aabb,
	const Eigen::MatrixBase<DerivedRS> &RS,
	const Eigen::MatrixBase<DerivedD> &D,
	const openCL_toolbox::openCL_params &params,
	const int maxHit,
	std::vector<std::vector<igl::Hit>> &hits);

} // namespace openCL_toolbox

#include "openCL_rayMeshIntersections.cpp"

#endif
