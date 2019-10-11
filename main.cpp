#include "openCL_toolbox/openCL_params.h"
#include "openCL_toolbox/openCL_util.h"
#include "openCL_toolbox/rayMeshIntersections/openCL_rayMeshIntersections.h"

#include <iostream>
#include <vector>
#include <chrono>

#pragma warning(push)
#pragma warning(disable : 4018 4129 4244 4267 4305 4566 4819 4996)
#include "igl/readPLY.h"
#include "igl/AABB.h"
#pragma warning(pop)

int main(int argv, char *argc[])
{
	////
	// openCL setup
	////
	// parameters for openCL computation
	openCL_toolbox::openCL_params params;

	// interactively select accelerator for openCL computation
	openCL_toolbox::selectAccelerator(params);

	// show detailed information for the accelerator
	openCL_toolbox::getAcceleratorDescription(params.accelerators);

	////
	// Example: ray--mesh intersection
	////
	// read mesh from file
	Eigen::MatrixXd V;
	Eigen::MatrixXi F;
	igl::readPLY(std::string(MESH_DIR) + "bunny.ply", V, F);

	// setup rays to be cast
	const int rayCount = 1000000;
	Eigen::MatrixXd RS, D;
	RS.resize(1000000, 3);
	RS.setRandom();
	D.resize(1000000, 3);
	D.setRandom();
	D.rowwise().normalize();

	// performance benchmark
	std::chrono::system_clock::time_point start, end;
	long long elapsed;

	// ray--mesh intersection with openCL (CPU, GPU, etc...)
	std::vector<std::vector<igl::Hit>> hitsCL;
	start = std::chrono::system_clock::now();
	openCL_toolbox::openCL_rayMeshIntersections(V, F, RS, D, params, 5, hitsCL);
	end = std::chrono::system_clock::now();
	elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << "openCL                         : " << elapsed << " ms" << std::endl;

	// AABB construction
	igl::AABB<Eigen::MatrixXd, 3> aabb;
	start = std::chrono::system_clock::now();
	aabb.init(V, F);
	end = std::chrono::system_clock::now();
	elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << "AABB construction              : " << elapsed << " ms" << std::endl;

	// ray--mesh intersection with openCL (with pre-computed AABB)
	start = std::chrono::system_clock::now();
	openCL_toolbox::openCL_rayMeshIntersections(V, F, aabb, RS, D, params, 5, hitsCL);
	end = std::chrono::system_clock::now();
	elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << "openCL (with pre-computed AABB): " << elapsed << " ms" << std::endl;

	// ray--mesh intersection with typical AABB (CPU, for comparison)
	std::vector<igl::Hit> hitsAABB;
	start = std::chrono::system_clock::now();
	for (int rIdx = 0; rIdx < RS.rows(); ++rIdx)
	{
		aabb.intersect_ray(V, F, RS.row(rIdx), D.row(rIdx), hitsAABB);
	}
	end = std::chrono::system_clock::now();
	elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << "CPU (AABB)                     : " << elapsed << " ms" << std::endl;

	return 0;
}
