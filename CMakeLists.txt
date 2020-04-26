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

SET(generated_src ${CMAKE_BINARY_DIR}/generated)
SET(generator ${CMAKE_SOURCE_DIR}/build-scripts/generate-code.py)
file(MAKE_DIRECTORY ${generated_src})
include_directories(AFTER ${generated_src})

SET(classes tags managers nodes properties tools objects serializers)
add_custom_target(registers)
foreach(R IN LISTS classes)
    SET(generated_cpp ${CMAKE_BINARY_DIR}/generated/register_${R}.cpp)
    SET(spec_file ${CMAKE_SOURCE_DIR}/lists/${R}.lst)
    add_custom_command(
        OUTPUT ${generated_cpp}
        COMMAND ${PYTHON_EXECUTABLE} ${generator} ${spec_file} ${generated_cpp}
        DEPENDS ${spec_file} ${generator}
        COMMENT "Generating ${R} register."
        VERBATIM
    )
    add_custom_target(${R}_register DEPENDS ${generated_cpp})
    add_dependencies(registers ${R}_register)
endforeach()

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
SET(qm_dir "${CMAKE_BINARY_DIR}/qm/")
SET(ts_dir "${CMAKE_CURRENT_SOURCE_DIR}/ts/")
file(MAKE_DIRECTORY ${qm_dir})

add_custom_command(
    OUTPUT "${qm_dir}/translations.qrc"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/translations.qrc"
            "${qm_dir}/translations.qrc"
    COMMENT "copy translations.qrc"
)

add_custom_target(translations_qm
    DEPENDS "${qm_dir}/translations.qrc"
)
set_source_files_properties("${qm_dir}/translations.qrc" PROPERTIES SKIP_AUTORCC ON)

foreach (lang de en es)
    SET(qm "${qm_dir}/omm_${lang}.qm")
    SET(ts "${ts_dir}/omm_${lang}.ts")
    add_custom_command(
        OUTPUT
          "${qm}"
          "${qm_dir}/qt_${lang}.qm"
          "${qm_dir}/qtbase_${lang}.qm"
          "${qm_dir}/qt_help_${lang}.qm"
          "${qm_dir}/qtlocation_${lang}.qm"
        DEPENDS ${TS_SOURCES}
        COMMAND ${Qt5_LRELEASE_EXECUTABLE} ${ts} -qm ${qm}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_QM_PATH}/qt_${lang}.qm" "${qm_dir}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_QM_PATH}/qtbase_${lang}.qm" "${qm_dir}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_QM_PATH}/qt_help_${lang}.qm" "${qm_dir}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_QM_PATH}/qtlocation_${lang}.qm" "${qm_dir}"
        COMMENT "Update prepare translation files ${lang}"
    )
    add_custom_target("qm_${lang}"
        DEPENDS ${qm}
                "${qm_dir}/qt_${lang}.qm"
                "${qm_dir}/qtbase_${lang}.qm"
                "${qm_dir}/qt_help_${lang}.qm"
                "${qm_dir}/qtlocation_${lang}.qm"
    )
    add_dependencies(translations_qm "qm_${lang}")
endforeach()

SET(qrc_translations_cpp "${CMAKE_CURRENT_BINARY_DIR}/qrc_translations.cpp")
set_property(SOURCE ${qm_dir}/translations.qrc PROPERTY SKIP_AUTOGEN ON)
set_property(SOURCE ${qrc_translations_cpp} PROPERTY SKIP_AUTOGEN ON)
add_custom_command(
    OUTPUT ${qrc_translations_cpp}
    COMMAND Qt5::rcc
      "${qm_dir}/translations.qrc"
      -o ${qrc_translations_cpp}
    COMMENT "Pack translations"
)
add_custom_target(translations_qrc DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qrc_translations.cpp")

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
