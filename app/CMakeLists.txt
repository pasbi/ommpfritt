find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-3.0)
link_directories(${GTKMM_LIBRARY_DIRS})
include_directories(${GTKMM_INCLUDE_DIRS})

find_package(Armadillo REQUIRED)
link_directories(${Armadillo_LIBRARY_DIRS})
include_directories(${Armadillo_INCLUDE_DIRS})

find_package(PythonLibs 3 REQUIRED)
link_directories(${PythonLibs_LIBRARY_DIRS})
include_directories(${PythonLibs_INCLUDE_DIRS})

find_package(pybind11 REQUIRED)
link_directories(${pybind11_LIBRARY_DIRS})
include_directories(${pybind11_INCLUDE_DIRS})

find_package(glog REQUIRED)
link_directories(${glog_LIBRARY_DIRS})
include_directories(${glog_INCLUDE_DIRS})


add_executable(ommpfritt
  application.cpp
  command.cpp
  commandstack.cpp
  hasproperties.cpp
  main.cpp
  mainwindow.cpp
  object.cpp
  objecttransformation.cpp
  objectview.cpp
  project.cpp
  property.cpp
  propertymanager.cpp
  propertyview.cpp
  python.cpp
  scene.cpp
  tag.cpp
  tagview.cpp
  view.cpp
  )


target_link_libraries(ommpfritt glog::glog)
target_link_libraries(ommpfritt ${GTKMM_LIBRARIES})
target_include_directories(ommpfritt PRIVATE ${ARMADILLO_INCLUDE_DIRS})
target_link_libraries(ommpfritt ${ARMADILLO_LIBRARIES})
target_include_directories(ommpfritt PRIVATE ${PYTHON_INCLUDE_DIRS})
target_link_libraries(ommpfritt python3.7m)
target_link_libraries(ommpfritt pybind11::embed)