#ifndef OPENCL_RAYMESHINTERSECTIONS_CPP
#define OPENCL_RAYMESHINTERSECTIONS_CPP
#define OCLSTRINGIFY(...) #__VA_ARGS__

#include "openCL_rayMeshIntersections.h"

#pragma warning(push)
#pragma warning(disable : 4018 4129 4244 4267 4305 4566 4819 4996)
#include "igl/per_face_normals.h"
#pragma warning(pop)

#include <unordered_set>

namespace openCL_toolbox
{

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
	std::vector<std::vector<igl::Hit>> &hits)
{
	igl::AABB<DerivedV, 3> aabb;
	aabb.init(V, F);
	openCL_rayMeshIntersections(
		V, F, aabb, RS, D, params, maxHit, hits);
}

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
			cl_V.at(v).x = static_cast<float>(V(v, 0));
			cl_V.at(v).y = static_cast<float>(V(v, 1));
			cl_V.at(v).z = static_cast<float>(V(v, 2));
		}

		const size_t aligned_FCount = static_cast<size_t>(std::ceil(static_cast<float>(F.rows()) / 4.0f) * 4.0f);
		std::vector<cl_int4> cl_F(aligned_FCount, {0, 0, 0, 0});
		Eigen::Matrix<typename DerivedV::Scalar, Eigen::Dynamic, Eigen::Dynamic> FN;
		igl::per_face_normals(V, F, FN);
		std::vector<cl_float4> cl_FN(aligned_FCount, {0.0f, 0.0f, 0.0f, 0.0f});
		for (int f = 0; f < F.rows(); ++f)
		{
			cl_F.at(f).x = static_cast<int>(F(f, 0));
			cl_F.at(f).y = static_cast<int>(F(f, 1));
			cl_F.at(f).z = static_cast<int>(F(f, 2));
			cl_FN.at(f).x = static_cast<float>(FN(f, 0));
			cl_FN.at(f).y = static_cast<float>(FN(f, 1));
			cl_FN.at(f).z = static_cast<float>(FN(f, 2));
		}

		const size_t aligned_RaySourcesCount = static_cast<size_t>(std::ceil(static_cast<float>(RS.rows()) / 4.0f) * 4.0f);
		std::vector<cl_float4> cl_R(aligned_RaySourcesCount, {0.0f, 0.0f, 0.0f, 0.0f});
		for (int r = 0; r < RS.rows(); ++r)
		{
			cl_R.at(r).x = static_cast<float>(RS(r, 0));
			cl_R.at(r).y = static_cast<float>(RS(r, 1));
			cl_R.at(r).z = static_cast<float>(RS(r, 2));
		}

		const size_t aligned_DCount = static_cast<size_t>(std::ceil(static_cast<float>(D.rows()) / 4.0f) * 4.0f);
		std::vector<cl_float4> cl_D(aligned_DCount, {0.0f, 0.0f, 0.0f, 0.0f});
		for (int d = 0; d < D.rows(); ++d)
		{
			Eigen::RowVectorXf D_normalized = D.row(d).template cast<float>().normalized();
			cl_D.at(d).x = D_normalized(0, 0);
			cl_D.at(d).y = D_normalized(0, 1);
			cl_D.at(d).z = D_normalized(0, 2);
		}
		// end setup.
		//////
		//////
		// construct AABBTree
		Eigen::Matrix<typename DerivedV::Scalar, Eigen::Dynamic, Eigen::Dynamic> bb_mins;
		Eigen::Matrix<typename DerivedV::Scalar, Eigen::Dynamic, Eigen::Dynamic> bb_maxs;
		Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic> elements;
		aabb.serialize(bb_mins, bb_maxs, elements);
		// construct serialized AABB
		const size_t aligned_BBCount = static_cast<size_t>(std::ceil(static_cast<float>(bb_mins.rows()) / 4.0f) * 4.0f);
		const size_t aligned_ElemCount = static_cast<size_t>(std::ceil(static_cast<float>(elements.rows()) / 16.0f) * 16.0f);
		std::vector<cl_float4> cl_bb_mins(aligned_BBCount, {0.0f, 0.0f, 0.0f, 0.0f});
		std::vector<cl_float4> cl_bb_maxs(aligned_BBCount, {0.0f, 0.0f, 0.0f, 0.0f});
		std::vector<cl_int> cl_elements(aligned_ElemCount, 0);
		for (int bb = 0; bb < bb_mins.rows(); ++bb)
		{
			cl_bb_mins.at(bb).x = static_cast<float>(bb_mins(bb, 0));
			cl_bb_mins.at(bb).y = static_cast<float>(bb_mins(bb, 1));
			cl_bb_mins.at(bb).z = static_cast<float>(bb_mins(bb, 2));
			cl_bb_maxs.at(bb).x = static_cast<float>(bb_maxs(bb, 0));
			cl_bb_maxs.at(bb).y = static_cast<float>(bb_maxs(bb, 1));
			cl_bb_maxs.at(bb).z = static_cast<float>(bb_maxs(bb, 2));
			cl_elements.at(bb) = elements(bb, 0);
		}
		//////

		//////
		// OpenCL setup and execution
		cl::Context context(params.accelerators);

		cl::Program::Sources sources;
		// do something for source
		const char source[] =
#include "kernel_computeIntersections.cl"
			;

		sources.push_back(std::make_pair(source, std::strlen(source)));

		cl::Program program = cl::Program(context, sources);
		try
		{
			program.build(params.accelerators);
		}
		catch (cl::Error &e)
		{
			if (e.err() == CL_BUILD_PROGRAM_FAILURE)
			{
				// Check the build status
				for (const auto &accelerator : params.accelerators)
				{

					cl_build_status status = program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(accelerator);

					// Get the build log
					std::string name = accelerator.getInfo<CL_DEVICE_NAME>();
					std::string buildlog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(accelerator);
					std::cerr << "Build log for " << name << ":" << std::endl;
					std::cerr << buildlog << std::endl;
				}

				exit(1);
			}
		}

		// because we need fixed length of buffer, we only store nearest maxHit hits.
		const size_t aligned_TCount = static_cast<size_t>(std::ceil(static_cast<float>(aligned_RaySourcesCount * maxHit) / 16.0f) * 16.0f);
		std::vector<cl_float4> cl_TUV(aligned_TCount, {0.0f, 0.0f, 0.0f, 0.0f});
		std::vector<cl_int> cl_Hit(aligned_TCount, 0);

		cl::Buffer buffer_V(context, (CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR), sizeof(cl_float4) * cl_V.size(), cl_V.data());
		cl::Buffer buffer_F(context, (CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR), sizeof(cl_int4) * cl_F.size(), cl_F.data());
		cl::Buffer buffer_FN(context, (CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR), sizeof(cl_float4) * cl_FN.size(), cl_FN.data());
		cl::Buffer buffer_R(context, (CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR), sizeof(cl_float4) * cl_R.size(), cl_R.data());
		cl::Buffer buffer_D(context, (CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR), sizeof(cl_float4) * cl_D.size(), cl_D.data());
		cl::Buffer buffer_BB_min(context, (CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR), sizeof(cl_float4) * cl_bb_mins.size(), cl_bb_mins.data());
		cl::Buffer buffer_BB_max(context, (CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR), sizeof(cl_float4) * cl_bb_maxs.size(), cl_bb_maxs.data());
		cl::Buffer buffer_elements(context, (CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR), sizeof(cl_int) * cl_elements.size(), cl_elements.data());
		cl::Buffer buffer_TUV(context, (CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR), sizeof(cl_float4) * cl_TUV.size(), cl_TUV.data());
		cl::Buffer buffer_Hit(context, (CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR), sizeof(cl_int) * cl_Hit.size(), cl_Hit.data());

		cl::Kernel computeIntersection_partial(program, "computeIntersection_partial");

		cl_int err;
		std::vector<cl::CommandQueue> queueVec;
		for (const auto &accelerator : params.accelerators)
		{
			queueVec.push_back(cl::CommandQueue(context, accelerator, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err));
		}
		int nextQueue = 0;

		// tweak chunkSize to be aligned!!
		// make sure that chunkSize * sizeof(cl_float) is aligned (64 byte)
		const size_t aligned_chunkSize = static_cast<size_t>(std::ceil(static_cast<float>(params.chunkSize) / 16.0f) * 16.0f);
		for (size_t offset = 0; offset < static_cast<size_t>(RS.rows()); offset += aligned_chunkSize)
		{
			const size_t alignedCount = std::min((aligned_RaySourcesCount - offset), aligned_chunkSize);
			const size_t rayCountToCompute = std::min((RS.rows() - offset), aligned_chunkSize);

			const cl_buffer_region tuv_partial{offset * maxHit * sizeof(cl_float4), alignedCount * maxHit * sizeof(cl_float4)};
			cl::Buffer buffer_TUV_partial = buffer_TUV.createSubBuffer((CL_MEM_WRITE_ONLY), CL_BUFFER_CREATE_TYPE_REGION, &tuv_partial);
			const cl_buffer_region hit_partial{offset * maxHit * sizeof(cl_int), alignedCount * maxHit * sizeof(cl_int)};
			cl::Buffer buffer_Hit_partial = buffer_Hit.createSubBuffer((CL_MEM_WRITE_ONLY), CL_BUFFER_CREATE_TYPE_REGION, &hit_partial);
			computeIntersection_partial.setArg(0, buffer_V);
			computeIntersection_partial.setArg(1, buffer_F);
			computeIntersection_partial.setArg(2, buffer_R);
			computeIntersection_partial.setArg(3, buffer_D);
			computeIntersection_partial.setArg(4, buffer_BB_min);
			computeIntersection_partial.setArg(5, buffer_BB_max);
			computeIntersection_partial.setArg(6, buffer_elements);
			computeIntersection_partial.setArg(7, maxHit);
			computeIntersection_partial.setArg(8, static_cast<int>(offset));
			computeIntersection_partial.setArg(9, static_cast<int>(elements.rows()));
			computeIntersection_partial.setArg(10, buffer_TUV_partial);
			computeIntersection_partial.setArg(11, buffer_Hit_partial);

			queueVec.at(nextQueue).enqueueNDRangeKernel(
				computeIntersection_partial, cl::NullRange, cl::NDRange(rayCountToCompute, 1), cl::NullRange);

			nextQueue = (nextQueue + 1) % queueVec.size();
		}

		for (const auto &queue : queueVec)
		{
			// not optimized...
			queue.finish();
		}

		cl_float4 *tuv_result;
		tuv_result = static_cast<cl_float4 *>(queueVec.at(0).enqueueMapBuffer(buffer_TUV, CL_TRUE, CL_MAP_READ, 0, sizeof(cl_float4) * cl_TUV.size()));
		cl_int *hit_result;
		hit_result = static_cast<cl_int *>(queueVec.at(0).enqueueMapBuffer(buffer_Hit, CL_TRUE, CL_MAP_READ, 0, sizeof(cl_int) * cl_Hit.size()));

		hits.clear();
		hits.resize(RS.rows());
		for (int r = 0; r < hits.size(); ++r)
		{
			for (int h = 0; h < maxHit; ++h)
			{
				if (tuv_result[r * maxHit + h].x > 0)
				{
					hits.at(r).push_back(igl::Hit{hit_result[r * maxHit + h], 0, tuv_result[r * maxHit + h].y, tuv_result[r * maxHit + h].z, tuv_result[r * maxHit + h].x});
				}
			}
		}
		queueVec.at(0).enqueueUnmapMemObject(buffer_TUV, tuv_result);
		queueVec.at(0).enqueueUnmapMemObject(buffer_Hit, hit_result);
	}
	catch (cl::Error err)
	{
		std::cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << std::endl;
	}
	return;
}
} // namespace openCL_toolbox

#endif