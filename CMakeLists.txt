cmake_minimum_required (VERSION 2.6)

project (ommpfritt)

set (CMAKE_AUTOMOC ON)
set (OMMPFRITT_VERSION_MAJOR 1)
set (OMMPFRITT_VERSION_MINOR 0)

configure_file (
  "${PROJECT_SOURCE_DIR}/ommpfrittconfig.h.in"
  "${PROJECT_BINARY_DIR}/ommpfrittconfig.h"
)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories("${PROJECT_BINARY_DIR}")

add_library(libommpfritt)
add_executable(ommpfritt)

target_compile_features(libommpfritt PUBLIC cxx_std_14)
target_compile_features(ommpfritt PUBLIC cxx_std_14)

find_package(PkgConfig)

find_package(Armadillo REQUIRED)
link_directories(${Armadillo_LIBRARY_DIRS})
include_directories(${Armadillo_INCLUDE_DIRS})
target_include_directories(libommpfritt PRIVATE ${ARMADILLO_INCLUDE_DIRS})
target_link_libraries(libommpfritt ${ARMADILLO_LIBRARIES})
target_link_libraries(ommpfritt ${ARMADILLO_LIBRARIES})

find_package(PythonLibs 3 REQUIRED)
link_directories(${PythonLibs_LIBRARY_DIRS})
include_directories(${PythonLibs_INCLUDE_DIRS})
target_include_directories(libommpfritt PRIVATE ${PYTHON_INCLUDE_DIRS})
target_link_libraries(libommpfritt python3.7m)
target_link_libraries(ommpfritt python3.7m)

find_package(pybind11 REQUIRED)
link_directories(${pybind11_LIBRARY_DIRS})
include_directories(${pybind11_INCLUDE_DIRS})
target_link_libraries(libommpfritt pybind11::embed)
target_link_libraries(ommpfritt pybind11::embed)

find_package(glog REQUIRED)
link_directories(${glog_LIBRARY_DIRS})
include_directories(${glog_INCLUDE_DIRS})
target_link_libraries(libommpfritt glog::glog)
target_link_libraries(ommpfritt glog::glog)

ADD_DEFINITIONS(-DQT_NO_KEYWORDS)
find_package(Qt5Widgets CONFIG REQUIRED)
target_link_libraries(libommpfritt Qt5::Widgets)
target_link_libraries(ommpfritt Qt5::Widgets)

add_subdirectory(src)
add_subdirectory(test)

target_link_libraries(ommpfritt libommpfritt)
