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

void getAcceleratorDescription(const std::vector<std::pair<cl::Platform, std::vector<cl::Device>>> &platforms_devices)
{
	std::vector<cl::Device> accelerators;
	for (const auto &p_d : platforms_devices)
	{
		for (const auto &d : p_d.second)
		{
			accelerators.push_back(d);
		}
	}
	getAcceleratorDescription(accelerators);
}

void getAccelerator(std::vector<std::pair<cl::Platform, std::vector<cl::Device>>> &platforms_devices)
{
	try
	{
		platforms_devices.clear();
		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);
		if (platforms.size() == 0)
		{
			std::cerr << "No platform found." << std::endl;
			return;
		}
		platforms_devices.resize(platforms.size());
		for (int pIdx = 0; pIdx < platforms.size(); ++pIdx)
		{
			const cl::Platform &platform = platforms.at(pIdx);
			platforms_devices.at(pIdx).first = platform;
			std::vector<cl::Device> &devices = platforms_devices.at(pIdx).second;
			platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
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

void selectAccelerator(const std::vector<std::pair<cl::Platform, std::vector<cl::Device>>> &platforms_devices, const int &platformIndex, const std::unordered_set<int> &acceleratorIndices, openCL_toolbox::openCL_params &params)
{
	params.accelerators.clear();
	for (int dIdx = 0; dIdx < platforms_devices.at(platformIndex).second.size(); ++dIdx)
	{
		if (acceleratorIndices.find(dIdx) != acceleratorIndices.end())
		{
			const cl::Device &device = platforms_devices.at(platformIndex).second.at(dIdx);
			params.accelerators.push_back(device);
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
	std::vector<std::pair<cl::Platform, std::vector<cl::Device>>> platforms_devices;
	getAccelerator(platforms_devices);

	// select platform
	for (int pIdx = 0; pIdx < platforms_devices.size(); ++pIdx)
	{
		const cl::Platform &platform = platforms_devices.at(pIdx).first;
		std::cout << pIdx << ": " << platform.getInfo<CL_PLATFORM_NAME>() << std::endl;
	}
	std::cout << std::endl;

	int platformIndex = -1;
	while (platformIndex < 0)
	{
		std::cout << "Please select platform you want to use: " << std::flush;

		std::string line;
		std::getline(std::cin, line);

		try
		{
			const int index = std::stoi(line);
			if (index >= 0 && index < platforms_devices.size())
			{
				platformIndex = index;
				break;
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

	// select device
	std::unordered_set<int> validIndices;
	const cl::Platform &platform = platforms_devices.at(platformIndex).first;
	std::cout << platform.getInfo<CL_PLATFORM_NAME>() << std::endl;
	for (int dIdx = 0; dIdx < platforms_devices.at(platformIndex).second.size(); ++dIdx)
	{
		const cl::Device &device = platforms_devices.at(platformIndex).second.at(dIdx);
		std::cout << "  " << dIdx << ": " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
		validIndices.insert(dIdx);
	}
	std::cout << std::endl;

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

	selectAccelerator(platforms_devices, platformIndex, acceleratorIndices, params);
}

} // namespace openCL_toolbox

#endif
