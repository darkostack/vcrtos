INCLUDE(CMakeForceCompiler)
# CROSS COMPILER SETTING
cmake_minimum_required (VERSION 3.5)
SET(CMAKE_SYSTEM_NAME Generic)

project(vcrtosKernel)

ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/include)
ADD_GLOBALDIR(${CMAKE_CURRENT_SOURCE_DIR}/source/core)

FILE(GLOB VCRTOS_KERNEL_SRC 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/vcrtos/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/core/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/core/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/soruce/core/*.cpp"
    )

# Library for providing the vcrtos-kernel functions.
CREATE_LIBRARY(vcrtosKernel "${VCRTOS_KERNEL_SRC}" "")
