#define OCLSTRINGIFY(...) #__VA_ARGS__

#include "openCL_computeIntersections.h"

#pragma warning(push)
#pragma warning(disable : 4018 4129 4244 4267 4305 4566 4819 4996)
#include "igl/per_face_normals.h"
#include "igl/AABB.h"
#pragma warning(pop)

#include <unordered_set>

////
// implementation
////
#define PI (3.14159265359)
//

void openCL_computeIntersections(
	const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> &V,
	const Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic> &F,
	const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> &raySources,
	const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> &dirs,
	const int &chunkSize,
	const cl::Device &device,
	std::vector<std::vector<igl::Hit>> &hits)
{
	try
	{
		//////
		// Note: due to memory alignment, I explicitly use float"4"
		//////
		//////
		// setup buffers for parallel computation
		const size_t aligned_VCount = static_cast<size_t>(std::ceil(static_cast<float>(V.rows()) / 4.0f) * 4.0f);
		std::vector<cl_float4> cl_V(aligned_VCount, {0.0f, 0.0f, 0.0f, 0.0f});
		for (int v = 0; v < V.rows(); ++v)
		{
			cl_V.at(v).x = V(v, 0);
			cl_V.at(v).y = V(v, 1);
			cl_V.at(v).z = V(v, 2);
		}

		const size_t aligned_FCount = static_cast<size_t>(std::ceil(static_cast<float>(F.rows()) / 4.0f) * 4.0f);
		std::vector<cl_int4> cl_F(aligned_FCount, {0, 0, 0, 0});
		Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> FN;
		igl::per_face_normals(V, F, FN);
		std::vector<cl_float4> cl_FN(aligned_FCount, {0.0f, 0.0f, 0.0f, 0.0f});
		for (int f = 0; f < F.rows(); ++f)
		{
			cl_F.at(f).x = F(f, 0);
			cl_F.at(f).y = F(f, 1);
			cl_F.at(f).z = F(f, 2);
			cl_FN.at(f).x = FN(f, 0);
			cl_FN.at(f).y = FN(f, 1);
			cl_FN.at(f).z = FN(f, 2);
		}

		const size_t aligned_RaySourcesCount = static_cast<size_t>(std::ceil(static_cast<float>(raySources.rows()) / 4.0f) * 4.0f);
		std::vector<cl_float4> cl_R(aligned_RaySourcesCount, {0.0f, 0.0f, 0.0f, 0.0f});
		for (int r = 0; r < raySources.rows(); ++r)
		{
			cl_R.at(r).x = raySources(r, 0);
			cl_R.at(r).y = raySources(r, 1);
			cl_R.at(r).z = raySources(r, 2);
		}

		const size_t aligned_DCount = static_cast<size_t>(std::ceil(static_cast<float>(dirs.rows()) / 4.0f) * 4.0f);
		std::vector<cl_float4> cl_D(aligned_DCount, {0.0f, 0.0f, 0.0f, 0.0f});
		for (int d = 0; d < dirs.rows(); ++d)
		{
			cl_D.at(d).x = dirs(d, 0);
			cl_D.at(d).y = dirs(d, 1);
			cl_D.at(d).z = dirs(d, 2);
		}
		// end setup.
		//////
		//////
		// construct AABBTree
		Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> bb_mins;
		Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> bb_maxs;
		Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic> elements;
		igl::AABB<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>, 3> aabb;
		aabb.init(V, F);
		aabb.serialize(bb_mins, bb_maxs, elements);
		// construct serialized AABB
		const size_t aligned_BBCount = static_cast<size_t>(std::ceil(static_cast<float>(bb_mins.rows()) / 4.0f) * 4.0f);
		const size_t aligned_ElemCount = static_cast<size_t>(std::ceil(static_cast<float>(elements.rows()) / 16.0f) * 16.0f);
		std::vector<cl_float4> cl_bb_mins(aligned_BBCount, {0.0f, 0.0f, 0.0f, 0.0f});
		std::vector<cl_float4> cl_bb_maxs(aligned_BBCount, {0.0f, 0.0f, 0.0f, 0.0f});
		std::vector<cl_int> cl_elements(aligned_ElemCount, 0);
		for (int bb = 0; bb < bb_mins.rows(); ++bb)
		{
			cl_bb_mins.at(bb).x = bb_mins(bb, 0);
			cl_bb_mins.at(bb).y = bb_mins(bb, 1);
			cl_bb_mins.at(bb).z = bb_mins(bb, 2);
			cl_bb_maxs.at(bb).x = bb_maxs(bb, 0);
			cl_bb_maxs.at(bb).y = bb_maxs(bb, 1);
			cl_bb_maxs.at(bb).z = bb_maxs(bb, 2);
			cl_elements.at(bb) = elements(bb, 0);
		}
		//////

		//////
		// OpenCL setup and execution
		cl::Context context({device});

		cl::Program::Sources sources;
		// do something for source
		const char source[] =
#include "kernel.cl"
			;

		sources.push_back(std::make_pair(source, std::strlen(source)));

		cl::Program program = cl::Program(context, sources);
		try
		{
			program.build({device});
		}
		catch (cl::Error &e)
		{
			if (e.err() == CL_BUILD_PROGRAM_FAILURE)
			{
				// Check the build status
				cl_build_status status = program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(device);

				// Get the build log
				std::string name = device.getInfo<CL_DEVICE_NAME>();
				std::string buildlog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
				std::cerr << "Build log for " << name << ":" << std::endl;
				std::cerr << buildlog << std::endl;

				exit(1);
			}
		}
		// if (program.build({device}) != CL_SUCCESS)
		// {
		// 	std::cout << " Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << "\n";
		// 	exit(1);
		// }

		// for our purpose, we only need nearest 3 intersections.
		// because if we have >= 3 intersections, that ray corresponds to violation.
		const size_t aligned_TCount = static_cast<size_t>(std::ceil(static_cast<float>(aligned_RaySourcesCount * 3) / 16.0f) * 16.0f);
		// intersection from outside: positive value
		// intersection from inside : negative value
		std::vector<cl_float> cl_T(aligned_TCount, 0.0f);
		std::vector<cl_int> cl_Hit(aligned_TCount, 0);

		cl::Buffer buffer_V(context, (CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR), sizeof(cl_float4) * cl_V.size(), cl_V.data());
		cl::Buffer buffer_F(context, (CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR), sizeof(cl_int4) * cl_F.size(), cl_F.data());
		cl::Buffer buffer_FN(context, (CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR), sizeof(cl_float4) * cl_FN.size(), cl_FN.data());
		cl::Buffer buffer_R(context, (CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR), sizeof(cl_float4) * cl_R.size(), cl_R.data());
		cl::Buffer buffer_D(context, (CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR), sizeof(cl_float4) * cl_D.size(), cl_D.data());
		cl::Buffer buffer_BB_min(context, (CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR), sizeof(cl_float4) * cl_bb_mins.size(), cl_bb_mins.data());
		cl::Buffer buffer_BB_max(context, (CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR), sizeof(cl_float4) * cl_bb_maxs.size(), cl_bb_maxs.data());
		cl::Buffer buffer_elements(context, (CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR), sizeof(cl_int) * cl_elements.size(), cl_elements.data());
		cl::Buffer buffer_T(context, (CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR), sizeof(cl_float) * cl_T.size(), cl_T.data());
		cl::Buffer buffer_Hit(context, (CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR), sizeof(cl_int) * cl_Hit.size(), cl_Hit.data());

		cl::Kernel computeIntersection_partial(program, "computeIntersection_partial");

		cl_int err;
		cl::CommandQueue queue(context, device, 0, &err);
		// tweak chunkSize to be aligned!!
		// make sure that chunkSize * sizeof(cl_float) is aligned (64 byte)
		const size_t aligned_chunkSize = static_cast<size_t>(std::ceil(static_cast<float>(chunkSize) / 16.0f) * 16.0f);
		for (size_t offset = 0; offset < static_cast<size_t>(raySources.rows()); offset += aligned_chunkSize)
		{
			const size_t alignedCount = std::min((aligned_RaySourcesCount - offset), aligned_chunkSize);
			const size_t rayCountToCompute = std::min((raySources.rows() - offset), aligned_chunkSize);

			const cl_buffer_region t_partial{offset * 3 * sizeof(cl_float), alignedCount * 3 * sizeof(cl_float)};
			cl::Buffer buffer_T_partial = buffer_T.createSubBuffer((CL_MEM_WRITE_ONLY), CL_BUFFER_CREATE_TYPE_REGION, &t_partial);
			const cl_buffer_region hit_partial{offset * 3 * sizeof(cl_int), alignedCount * 3 * sizeof(cl_int)};
			cl::Buffer buffer_Hit_partial = buffer_Hit.createSubBuffer((CL_MEM_WRITE_ONLY), CL_BUFFER_CREATE_TYPE_REGION, &hit_partial);
			computeIntersection_partial.setArg(0, buffer_V);
			computeIntersection_partial.setArg(1, buffer_F);
			computeIntersection_partial.setArg(2, buffer_R);
			computeIntersection_partial.setArg(3, buffer_D);
			computeIntersection_partial.setArg(4, buffer_BB_min);
			computeIntersection_partial.setArg(5, buffer_BB_max);
			computeIntersection_partial.setArg(6, buffer_elements);
			computeIntersection_partial.setArg(7, static_cast<int>(offset));
			computeIntersection_partial.setArg(8, static_cast<int>(elements.rows()));
			computeIntersection_partial.setArg(9, buffer_T_partial);
			computeIntersection_partial.setArg(10, buffer_Hit_partial);

			queue.enqueueNDRangeKernel(
				computeIntersection_partial, cl::NullRange, cl::NDRange(rayCountToCompute, 1), cl::NullRange);

			queue.finish();
		}
		std::cout << "intersection enqueued." << std::endl;

		cl_float *t_result;
		t_result = static_cast<cl_float *>(queue.enqueueMapBuffer(buffer_T, CL_TRUE, CL_MAP_READ, 0, sizeof(cl_float) * cl_T.size()));
		cl_int *hit_result;
		hit_result = static_cast<cl_int *>(queue.enqueueMapBuffer(buffer_Hit, CL_TRUE, CL_MAP_READ, 0, sizeof(cl_int) * cl_Hit.size()));

		hits.clear();
		hits.resize(raySources.rows());
		for (int r = 0; r < hits.size(); ++r)
		{
			for (int h = 0; h < 3; ++h)
			{
				if (t_result[r * 3 + h] > 0)
				{
					hits.at(r).push_back(igl::Hit{hit_result[r * 3 + h], 0, 0.0f, 0.0f, t_result[r * 3 + h]});
				}
			}
		}
		queue.enqueueUnmapMemObject(buffer_T, t_result);
		queue.enqueueUnmapMemObject(buffer_Hit, hit_result);

		std::cout << "intersection done." << std::endl;
	}
	catch (cl::Error err)
	{
		std::cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << std::endl;
	}
	return;
}
