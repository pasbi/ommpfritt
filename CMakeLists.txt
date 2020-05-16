cmake_minimum_required (VERSION 3.13)

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
option(BUILD_TESTS "build the unit tests." ON)

find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5 COMPONENTS LinguistTools)
find_package(PkgConfig)
find_package(Python3 3.7 REQUIRED COMPONENTS Interpreter Development)
find_package(KF5ItemModels REQUIRED)
find_package(pybind11 REQUIRED)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/" ${CMAKE_MODULE_PATH})
include(generate_registers)
include(generate_translations)
include(generate_icons)
include(generate_resources)
if (BUILD_TESTS)
    include(DownloadGoogleTest)
endif()

list(APPEND classes tags managers nodes properties tools objects serializers)
generate_registers("${classes}")

set (QT_QM_PATH CACHE PATH
    "path where to find the Qt translation files (qt_en.qm, qtbase_de.qm, etc.")

configure_file (
  "${PROJECT_SOURCE_DIR}/ommpfrittconfig.h.in"
  "${PROJECT_BINARY_DIR}/ommpfrittconfig.h"
)

ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

set(ts_dir "${CMAKE_CURRENT_SOURCE_DIR}/ts/")
set(qrc_translations_cpp "${CMAKE_CURRENT_BINARY_DIR}/qrc_translations.cpp")
set(languages de en es)
set(translations_qrc "${CMAKE_CURRENT_BINARY_DIR}/qm/translations.qrc")
set(prefixes omm qtlocation qt_help qt)
set(cfg_files
  ${CMAKE_CURRENT_SOURCE_DIR}/uicolors/ui-colors-light.cfg
  ${CMAKE_CURRENT_SOURCE_DIR}/uicolors/ui-colors-dark.cfg
  ${CMAKE_CURRENT_SOURCE_DIR}/keybindings/default_keybindings.cfg
)
generate_translations("${translations_qrc}" "${ts_dir}" "${languages}" "${prefixes}" "${cfg_files}")

set(qrc_files_cli
    "${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc"
    ${translations_qrc}
)
set(compiled_resource_file_cli ${CMAKE_CURRENT_BINARY_DIR}/qrc_resources_cli.cpp)
generate_resources(resources_cli ${compiled_resource_file_cli} "${qrc_files_cli}")

set(icons_qrc "${CMAKE_CURRENT_BINARY_DIR}/icons/icons.qrc")
if (GENERATE_ICONS)
  generate_icons("${icons_qrc}" "${classes};extra")
endif()

set(qrc_files "${qrc_files_cli}")
if (GENERATE_ICONS)
  list(APPEND qrc_files ${icons_qrc})
endif()
set(compiled_resource_file ${CMAKE_CURRENT_BINARY_DIR}/qrc_resources.cpp)
generate_resources(resources ${compiled_resource_file} "${qrc_files}")

macro(set_warning_level target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /WX)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -pedantic -Werror)
  endif()
endmacro()

add_executable(ommpfritt-cli src/maincli.cpp "${compiled_resource_file_cli}")
add_executable(ommpfritt src/main.cpp "${compiled_resource_file}")
add_executable(ommpfritt_unit_tests)
add_library(libommpfritt STATIC)

set_warning_level(ommpfritt-cli)
set_warning_level(ommpfritt)
set_warning_level(ommpfritt_unit_tests)
set_warning_level(libommpfritt)

if (GENERATE_ICONS)
  add_dependencies(resources ommpfritt-cli)
  add_dependencies(icons_png ommpfritt-cli)
endif()
add_dependencies(resources_cli translations_qm)
add_dependencies(resources translations_qm)
add_dependencies(libommpfritt registers)
add_dependencies(ommpfritt libommpfritt resources)
add_dependencies(ommpfritt_unit_tests resources_cli)
add_dependencies(ommpfritt-cli libommpfritt resources_cli)

target_include_directories(libommpfritt PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${PROJECT_BINARY_DIR}"
)
target_compile_features(libommpfritt PUBLIC cxx_std_17)
target_link_libraries(libommpfritt Python3::Python)
target_link_libraries(libommpfritt pybind11::embed)
target_link_libraries(libommpfritt Qt5::Widgets Qt5::Svg)
target_link_libraries(libommpfritt poppler-qt5)
target_link_libraries(libommpfritt -lpthread -lm)
target_link_libraries(libommpfritt KF5ItemModels)
target_link_libraries(ommpfritt libommpfritt)
target_link_libraries(ommpfritt-cli libommpfritt)
target_link_libraries(ommpfritt_unit_tests libommpfritt)
target_link_libraries(ommpfritt_unit_tests gtest)
target_link_libraries(ommpfritt_unit_tests gtest_main)

if (WIN32)
  target_link_libraries(libommpfritt -lopengl32)
else()
  target_link_libraries(libommpfritt -ldl -lutil)
  target_link_libraries(libommpfritt -lGL)
endif()

add_subdirectory(src)
add_subdirectory(test)

install(
    TARGETS ommpfritt
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static
)

install(
    TARGETS libommpfritt
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static
)

