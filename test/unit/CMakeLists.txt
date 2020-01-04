FILE(GLOB SRC_FILES
  "color.cpp"
  "cubic.cpp"
  "dnftest.cpp"
  "geometry.cpp"
  "path.cpp"
  "propertytest.cpp"
  "main.cpp"
  "tree.cpp"
)

# Add executable
add_executable(ommpfritt_unit_tests ${SRC_FILES})

# Link ommpfritt
target_link_libraries(ommpfritt_unit_tests libommpfritt)

# Link gtest
target_link_libraries(ommpfritt_unit_tests gtest)
target_link_libraries(ommpfritt_unit_tests gtest_main)
include_directories(${gtest_SOURCE_DIR}/include)
