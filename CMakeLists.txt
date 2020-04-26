cmake_minimum_required (VERSION 2.8.4)  # warnings vanish with Version > 3.3

# Suppresses a warning in cmake 3.x concerning a currently unused feature (setting VERSION in project() command)
if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

project (ommpfritt)

set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)
set (CMAKE_AUTOUIC ON)
set (OMMPFRITT_VERSION_MAJOR 1)
set (OMMPFRITT_VERSION_MINOR 0)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_registers.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_translations.cmake)

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

file(GLOB_RECURSE TS_SOURCES "*.cpp" "*.h" "*.ui")
SET(ts_dir "${CMAKE_CURRENT_SOURCE_DIR}/ts/")
SET(qrc_translations_cpp "${CMAKE_CURRENT_BINARY_DIR}/qrc_translations.cpp")
list(APPEND languages de en es)

generate_translations("${qrc_translations_cpp}" "${ts_dir}" "${languages}")

add_library(libommpfritt)
add_dependencies(translations_qrc translations_qm)
add_dependencies(libommpfritt registers translations_qrc)
SET(RESOURCES "${qrc_translations_cpp}" resources.qrc)
add_executable(ommpfritt src/main.cpp ${RESOURCES})
add_dependencies(ommpfritt libommpfritt)
add_executable(ommpfritt-cli src/maincli.cpp ${RESOURCES})
add_dependencies(ommpfritt-cli libommpfritt)

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
