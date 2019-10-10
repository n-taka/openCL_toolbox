#include "openCL_toolbox/openCL_params.h"
#include "openCL_toolbox/openCL_util.h"

#include <iostream>
#include <string>
#include <vector>

int main(int argv, char *argc[])
{
	openCL_toolbox::openCL_params params;

	openCL_toolbox::selectAccelerator(params);
	openCL_toolbox::getAcceleratorDescription(params.accelerators);

	return 0;
}
