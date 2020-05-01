cmake_minimum_required (VERSION 3.12)

# Suppresses a warning in cmake 3.x concerning a currently unused feature (setting VERSION in project() command)
if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

project (ommpfritt)

set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTOUIC ON)
set (OMMPFRITT_VERSION_MAJOR 1)
set (OMMPFRITT_VERSION_MINOR 0)

option(GENERATE_ICONS "There will be no icons in the application if this option is disabled." ON)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_registers.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_translations.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_icons.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_resources.cmake)

find_package(Python3 3.7 REQUIRED COMPONENTS Interpreter)

list(APPEND classes tags managers nodes properties tools objects serializers)
generate_registers("${classes}")

set (QT_QM_PATH CACHE PATH
    "path where to find the Qt translation files (qt_en.qm, qtbase_de.qm, etc.")

configure_file (
  "${PROJECT_SOURCE_DIR}/ommpfrittconfig.h.in"
  "${PROJECT_BINARY_DIR}/ommpfrittconfig.h"
)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories("${PROJECT_BINARY_DIR}")

find_package(PkgConfig)

find_package(PythonLibs 3 REQUIRED)
link_directories(${PythonLibs_LIBRARY_DIRS})
include_directories(${PythonLibs_INCLUDE_DIRS})

find_package(pybind11 REQUIRED)
link_directories(${pybind11_LIBRARY_DIRS})
include_directories(${pybind11_INCLUDE_DIRS})

ADD_DEFINITIONS(-DQT_NO_KEYWORDS)
find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5 COMPONENTS LinguistTools)

find_package(KF5ItemModels REQUIRED)

SET(ts_dir "${CMAKE_CURRENT_SOURCE_DIR}/ts/")
SET(qrc_translations_cpp "${CMAKE_CURRENT_BINARY_DIR}/qrc_translations.cpp")
list(APPEND languages de en es)

set(translations_qrc "${CMAKE_CURRENT_BINARY_DIR}/qm/translations.qrc")
set(prefixes omm qtlocation qt_help qt)
generate_translations("${translations_qrc}" "${ts_dir}" "${languages}" "${prefixes}")

set(qrc_files_cli
    "${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc"
    ${translations_qrc}
)
set(compiled_resource_file_cli ${CMAKE_CURRENT_BINARY_DIR}/qrc_resources_cli.cpp)
generate_resources(resources_cli ${compiled_resource_file_cli} "${qrc_files_cli}")
add_dependencies(resources_cli translations_qm)

add_library(libommpfritt)
add_dependencies(libommpfritt registers)

add_executable(ommpfritt-cli src/maincli.cpp "${compiled_resource_file_cli}")
add_dependencies(ommpfritt-cli libommpfritt resources_cli)


set(icons_qrc "${CMAKE_CURRENT_BINARY_DIR}/icons/icons.qrc")
if (GENERATE_ICONS)
  generate_icons("${icons_qrc}" "${classes};extra")
  add_dependencies(icons_png ommpfritt-cli)
endif()

set(qrc_files "${qrc_files_cli}")
if (GENERATE_ICONS)
  list(APPEND qrc_files ${icons_qrc})
endif()
set(compiled_resource_file ${CMAKE_CURRENT_BINARY_DIR}/qrc_resources.cpp)
generate_resources(resources ${compiled_resource_file} "${qrc_files}")
add_dependencies(resources translations_qm)

if (GENERATE_ICONS)
  add_dependencies(resources ommpfritt-cli)
endif()

add_executable(ommpfritt src/main.cpp "${compiled_resource_file}")
add_dependencies(ommpfritt libommpfritt resources)

target_compile_features(libommpfritt PUBLIC cxx_std_17)
target_link_libraries(libommpfritt ${PYTHON_LIBRARIES})
target_link_libraries(libommpfritt pybind11::embed)
target_link_libraries(libommpfritt Qt5::Widgets Qt5::Svg)
target_link_libraries(libommpfritt poppler-qt5)
target_link_libraries(libommpfritt -lpthread -lm)
target_link_libraries(libommpfritt KF5ItemModels)
if (WIN32)
  target_link_libraries(libommpfritt -lopengl32)
else()
  target_link_libraries(libommpfritt -ldl -lutil)
  target_link_libraries(libommpfritt -lGL)
endif()

add_subdirectory(src)
add_subdirectory(test)

target_link_libraries(ommpfritt libommpfritt)
target_link_libraries(ommpfritt-cli libommpfritt)

install(TARGETS ommpfritt RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib/static)
install(TARGETS libommpfritt RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib/static)

