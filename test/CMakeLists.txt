enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

include_directories("${CMAKE_SOURCE_DIR}/src")
add_subdirectory(unit)
