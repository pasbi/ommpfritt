target_sources(ommpfritt_unit_tests PRIVATE
  color.cpp
  common.cpp
  dnftest.cpp
  geometry.cpp
  pathtest.cpp
  propertytest.cpp
  main.cpp
  splinetypetest.cpp
  stringinterpolation.cpp
  tree.cpp
  iotest.cpp
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/testconfig.h)
target_include_directories(ommpfritt_unit_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_sources(ommpfritt_unit_tests PRIVATE "${compiled_resource_file_cli}")
