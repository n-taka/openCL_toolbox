#include "openCL_toolbox/openCL_params.h"
#include "openCL_toolbox/openCL_util.h"
#include "openCL_toolbox/rayMeshIntersections/openCL_rayMeshIntersections.h"

#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#pragma warning(push)
#pragma warning(disable : 4018 4129 4244 4267 4305 4566 4819 4996)
#include "igl/readPLY.h"
#include "igl/AABB.h"
#pragma warning(pop)

int main(int argv, char *argc[])
{
	openCL_toolbox::openCL_params params;

	openCL_toolbox::selectAccelerator(params);
	openCL_toolbox::getAcceleratorDescription(params.accelerators);

	Eigen::MatrixXd V;
	Eigen::MatrixXi F;
	igl::readPLY("bunny_color.ply", V, F);

	const int rayCount = 1000000;
	Eigen::MatrixXd RS, D;
	RS.resize(1000000, 3);
	RS.setRandom();
	D.resize(1000000, 3);
	D.setRandom();
	D.rowwise().normalize();

	std::vector<std::vector<igl::Hit>> hits;

	std::chrono::system_clock::time_point start, end;
	double elapsed;

	start = std::chrono::system_clock::now();
	openCL_toolbox::openCL_rayMeshIntersections(V, F, RS, D, params, 5, hits);
	end = std::chrono::system_clock::now();
	elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << "openCL: " << elapsed << " ms" << std::endl;

	// for (int rIdx = 0; rIdx < hits.size(); ++rIdx)
	// {
	// 	std::cout << rIdx << std::endl;
	// 	for (const auto &h : hits.at(rIdx))
	// 	{
	// 		std::cout << h.id << " " << h.u << " " << h.v << " " << h.t << std::endl;
	// 	}
	// }
	// std::cout << std::endl
	// 		  << std::endl;

	igl::AABB<Eigen::MatrixXd, 3> aabb;
	start = std::chrono::system_clock::now();
	aabb.init(V, F);
	std::vector<igl::Hit> hitAABB;
	for (int rIdx = 0; rIdx < hits.size(); ++rIdx)
	{
		// std::cout << rIdx << std::endl;
		aabb.intersect_ray(V, F, RS.row(rIdx), D.row(rIdx), hitAABB);
		// for (const auto &h : hitAABB)
		// {
		// 	std::cout << h.id << " " << h.u << " " << h.v << " " << h.t << std::endl;
		// }
	}
	end = std::chrono::system_clock::now();
	elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << "AABB: " << elapsed << " ms" << std::endl;

	return 0;
}
