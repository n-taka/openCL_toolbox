cmake_minimum_required(VERSION 3.7)
set(PROJECT openCL_toolbox)
project(${PROJECT})
# version
set(serial "1.0")
# basically, we use Release
if (NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release")
endif()

########################
##### Dependencies #####
########################
####
# for windows (find installed packages via vcpkg)
# cmake -DCMAKE_TOOLCHAIN_FILE=C:/src/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows ..
####
####
# for linux (find installed packages via vcpkg)
# cmake -DCMAKE_TOOLCHAIN_FILE=/usr/src/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-linux ..
####
####
# for mac (find installed packages via homebrew)
# TBA!!
####

###
# eigen3
###
find_package(Eigen3 3.3 REQUIRED)

###
# OpenCL
###
find_package(OpenCL REQUIRED)

# Include directories for this project
set(INCLUDE_DIR
    ${EIGEN3_INCLUDE_DIR}
    ${OpenCL_INCLUDE_DIRS}
    include
    submodule/libigl/include
)

set(CPP_FILES
    main.cpp
)

set(LINK_LIBS
    ${OpenCL_LIBRARIES}
)

# Target for this build
add_executable(${PROJECT} ${CPP_FILES})

# Add a include files
target_include_directories(${PROJECT} PUBLIC ${INCLUDE_DIR})
# Add external libraries
target_link_libraries(${PROJECT} PRIVATE ${LINK_LIBS})

set(
    MY_DEBUG_OPTIONS
    -g
)
set(
    MY_RELEASE_OPTIONS
    -O2
)
target_compile_options(${PROJECT} PUBLIC "$<$<CONFIG:DEBUG>:${MY_DEBUG_OPTIONS}>")
target_compile_options(${PROJECT} PUBLIC "$<$<CONFIG:RELEASE>:${MY_RELEASE_OPTIONS}>")
target_compile_features(${PROJECT} PUBLIC cxx_std_11)
