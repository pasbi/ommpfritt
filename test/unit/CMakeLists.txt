FILE(GLOB SRC_FILES
  "color.cpp"
  "cubic.cpp"
  "common.cpp"
  "dnftest.cpp"
  "geometry.cpp"
  "application.cpp"
  "path.cpp"
  "propertytest.cpp"
  "main.cpp"
  "splinetypetest.cpp"
  "tree.cpp"
  "toolbartest.cpp"
)

target_sources(ommpfritt_unit_tests PRIVATE ${SRC_FILES} ${compiled_resource_file_cli})
include_directories(${gtest_SOURCE_DIR}/include)
