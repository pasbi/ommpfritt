cmake_minimum_required (VERSION 2.6)

project (ommpfritt)

set (OMMPFRITT_VERSION_MAJOR 1)
set (OMMPFRITT_VERSION_MINOR 0)
 
configure_file (
  "${PROJECT_SOURCE_DIR}/ommpfrittconfig.h.in"
  "${PROJECT_BINARY_DIR}/ommpfrittconfig.h"
  )

include_directories("${PROJECT_BINARY_DIR}")

add_subdirectory(lib)
add_subdirectory(app)

