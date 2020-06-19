target_sources(ommpfritt_unit_tests PRIVATE
  color.cpp
  common.cpp
  dnftest.cpp
  geometry.cpp
  application.cpp
  pathtest.cpp
  propertytest.cpp
  main.cpp
  splinetypetest.cpp
  tree.cpp
  toolbartest.cpp
)

include_directories(${gtest_SOURCE_DIR}/include)
