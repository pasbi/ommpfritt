cmake_minimum_required (VERSION 2.8.4)  # warnings vanish with Version > 3.3

project (ommpfritt)

set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)
set (CMAKE_AUTOUIC ON)
set (OMMPFRITT_VERSION_MAJOR 1)
set (OMMPFRITT_VERSION_MINOR 0)
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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/translations.qrc ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
set(RESOURCES resources.qrc ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc)

file (GLOB TS_FILES
  ts/omm_en.ts
  ts/omm_de.ts
)

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

foreach (lang en de)
  foreach (pref qt qtbase qt_help qtlocation)
    file(COPY ${QT_QM_PATH}/${pref}_${lang}.qm DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
  endforeach(pref)
endforeach(lang)

add_library(libommpfritt)

add_executable(ommpfritt ${RESOURCES})

target_compile_features(libommpfritt PUBLIC cxx_std_17)
target_compile_features(ommpfritt PUBLIC cxx_std_17)

target_link_libraries(libommpfritt python3.7m)
target_link_libraries(ommpfritt python3.7m)
target_include_directories(libommpfritt PRIVATE ${PYTHON_INCLUDE_DIRS})
target_link_libraries(libommpfritt pybind11::embed)
target_link_libraries(ommpfritt pybind11::embed)
target_link_libraries(libommpfritt Qt5::Widgets Qt5::Svg)
target_link_libraries(ommpfritt Qt5::Widgets Qt5::Svg)
target_link_libraries(libommpfritt poppler-qt5)
target_link_libraries(ommpfritt poppler-qt5)
target_link_libraries(libommpfritt -lpthread -lm -ldl -lutil)
target_link_libraries(ommpfritt -lpthread -lm -ldl -lutil)

add_subdirectory(src)
add_subdirectory(test)

target_link_libraries(ommpfritt libommpfritt)
