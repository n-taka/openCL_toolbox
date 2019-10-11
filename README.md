# openCL_toolbox
light-weight openCL toolbox for geometry processing

## Dependencies
- [OpenCL](https://www.khronos.org/opencl/)
- [Eigen](http://eigen.tuxfamily.org/index.php?title=Main_Page)
- [libigl](https://libigl.github.io/)
    - actually, we use forked one (https://github.com/n-taka/libigl.git)

## Installation
### Windows
```
# Repository itself and submodules (libigl)
git clone --recursive https://github.com/n-taka/openCL_toolbox.git

# Other dependencies (eigen)
vcpkg install eigen3:x64-windows
```
The instruction for installing [vcpkg](https://github.com/Microsoft/vcpkg) is found in [their repository](https://github.com/Microsoft/vcpkg)

### macOS
```
# Repository itself and submodules (libigl)
git clone --recursive https://github.com/n-taka/openCL_toolbox.git

# Other dependencies (eigen)
brew install eigen
```
The instruction for installing [homebrew](https://brew.sh/) is found in [their website](https://brew.sh/)

## Build and run
### Windows
We use [cmake](https://cmake.org/) for build.
```
cd /path/to/repository/root
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows ..
cmake --build . --config "Release"
./Release/openCL_toolbox.exe
```

### macOS
We use [cmake](https://cmake.org/) for build.
```
cd /path/to/repository/root
mkdir build
cd build
cmake ..
cmake --build . --config "Release"
./openCL_toolbox.exe
```

## Use this tool for your project
For detail, please check CMakeLists.txt and main.cpp

## Notes
Compilation and execution on macOS is not yet tested... Sorry!

## License
MPLv2. Please see LICENSE

## Contact
Kazutaka Nakashima [kazutaka.nakashima@n-taka.info](mailto:kazutaka.nakashima@n-taka.info)
