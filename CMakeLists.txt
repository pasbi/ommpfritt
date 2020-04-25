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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/translations.qrc ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
set(RESOURCES resources.qrc ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc)

file (GLOB TS_FILES
  ts/omm_de.ts
  ts/omm_en.ts
  ts/omm_es.ts
)

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

foreach (lang de en es)
  foreach (pref qt qtbase qt_help qtlocation)
    file(COPY ${QT_QM_PATH}/${pref}_${lang}.qm DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
  endforeach(pref)
endforeach(lang)


add_library(libommpfritt)
add_dependencies(libommpfritt registers)
add_executable(ommpfritt ${RESOURCES})
add_executable(ommpfritt-cli ${RESOURCES})

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
