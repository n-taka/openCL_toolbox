#ifndef OPENCL_UTIL_CPP
#define OPENCL_UTIL_CPP
#include "cl.hpp"
#include "openCL_Util.h"

#include <vector>
#include <iostream>
#include <sstream>

////
// implementation
////
namespace openCL_toolbox
{
void getAcceleratorDescription(const std::vector<cl::Device> &accelerators)
{
	for (const auto &accelerator : accelerators)
	{
		std::cout << accelerator.getInfo<CL_DEVICE_NAME>() << std::endl;
		std::cout << "Device vender  : ";
		std::cout << accelerator.getInfo<CL_DEVICE_VENDOR>() << std::endl;
		std::cout << "Device version : ";
		std::cout << accelerator.getInfo<CL_DEVICE_VERSION>() << std::endl;
		std::cout << "Driver version : ";
		std::cout << accelerator.getInfo<CL_DRIVER_VERSION>() << std::endl;
		std::cout << "Extensions     : ";
		std::cout << accelerator.getInfo<CL_DEVICE_EXTENSIONS>() << std::endl;
		std::cout << std::endl;
	}
	return;
}

void getAcceleratorDescription(const std::unordered_map<std::string, std::vector<std::pair<int, cl::Device>>> &platform_devices, const int uniqueIdx)
{
	std::vector<cl::Device> accelerators;
	for (const auto &p_d : platform_devices)
	{
		for (const auto &id_d : p_d.second)
		{
			if (uniqueIdx < 0 || uniqueIdx == id_d.first)
			{
				accelerators.push_back(id_d.second);
			}
		}
	}
	getAcceleratorDescription(accelerators);
}

void getAccelerator(std::unordered_map<std::string, std::vector<std::pair<int, cl::Device>>> &platform_devices)
{
	try
	{
		platform_devices.clear();
		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);
		if (platforms.size() == 0)
		{
			std::cerr << "No platform found." << std::endl;
			return;
		}
		int uniqueDeviceIndex = 0;
		for (auto &platform : platforms)
		{
			std::vector<std::pair<int, cl::Device>> &deviceList = platform_devices[platform.getInfo<CL_PLATFORM_NAME>()];
			std::vector<cl::Device> devices;
			platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
			for (auto &device : devices)
			{
				deviceList.push_back(std::make_pair(uniqueDeviceIndex++, device));
			}
		}
	}
	catch (cl::Error const &ex)
	{
		std::cerr << "OpenCL Error: " << ex.what() << " (code " << ex.err() << ")" << std::endl;
		return;
	}
	catch (std::exception const &ex)
	{
		std::cerr << "Exception: " << ex.what() << std::endl;
		return;
	}
}

void selectAccelerator(const std::unordered_map<std::string, std::vector<std::pair<int, cl::Device>>> &platform_devices, const std::unordered_set<int> &acceleratorIndices, openCL_toolbox::openCL_params &params)
{
	params.accelerators.clear();
	for (const auto &p_d : platform_devices)
	{
		for (const auto &id_d : p_d.second)
		{
			if (acceleratorIndices.find(id_d.first) != acceleratorIndices.end())
			{
				params.accelerators.push_back(id_d.second);
			}
		}
	}
	if (params.accelerators.size() == 0)
	{
		std::cerr << "No valid accelerator is selected. We use default one." << std::endl;
		params.accelerators.push_back(cl::Device::getDefault());
	}
}

void selectAccelerator(openCL_toolbox::openCL_params &params)
{
	std::unordered_map<std::string, std::vector<std::pair<int, cl::Device>>> platform_devices;
	getAccelerator(platform_devices);

	std::unordered_set<int> validIndices;
	for (const auto &p_d : platform_devices)
	{
		std::cout << p_d.first << std::endl;
		for (const auto &id_d : p_d.second)
		{
			std::cout << "  " << id_d.first << ": " << id_d.second.getInfo<CL_DEVICE_NAME>() << std::endl;
			validIndices.insert(id_d.first);
		}
		std::cout << std::endl;
	}

	std::unordered_set<int> acceleratorIndices;
	while (acceleratorIndices.size() == 0)
	{
		std::cout << "Please select devices you want to use (comma separated): " << std::flush;

		std::string line;
		std::getline(std::cin, line);

		std::stringstream linestream(line);
		std::string value;
		while (getline(linestream, value, ','))
		{
			try
			{
				const int index = std::stoi(value);
				if (validIndices.find(index) != validIndices.end())
				{
					acceleratorIndices.insert(index);
				}
				else
				{
					std::cerr << "Please input valid number." << std::endl;
				}
			}
			catch (std::exception &)
			{
				std::cerr << "Please input valid number." << std::endl;
			}
		}
	}

	selectAccelerator(platform_devices, acceleratorIndices, params);
}

} // namespace openCL_toolbox

#endif
