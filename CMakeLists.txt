cmake_minimum_required (VERSION 3.13)

# Suppresses a warning in cmake 3.x concerning a currently unused feature (setting VERSION in project() command)
if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

project(ommpfritt VERSION 0.1.2)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TRANSLATION "Build translations of the application." ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Svg LinguistTools)
find_package(PkgConfig)
find_package(Python3 3.6 REQUIRED COMPONENTS Interpreter Development)
find_package(KF5ItemModels REQUIRED)
find_package(2Geom REQUIRED)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/" ${CMAKE_MODULE_PATH})
include(generate_registers)
if (BUILD_TRANSLATION)
  include(generate_translations)
endif()
include(generate_resources)
include(GetGitRevisionDescription)
git_describe(GIT_DESCRIBE "--tags")

list(APPEND classes tags managers nodes properties tools objects)
generate_registers("${classes}")

ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

if (BUILD_TRANSLATION)
  set(ts_dir "${CMAKE_CURRENT_SOURCE_DIR}/ts/")
  set(qrc_translations_cpp "${CMAKE_CURRENT_BINARY_DIR}/qrc_translations.cpp")
  set(languages de en es)
  set(translations_qrc "${CMAKE_CURRENT_BINARY_DIR}/qm/translations.qrc")
  set(cfg_files
    ${CMAKE_CURRENT_SOURCE_DIR}/uicolors/ui-colors-light.cfg
    ${CMAKE_CURRENT_SOURCE_DIR}/uicolors/ui-colors-dark.cfg
    ${CMAKE_CURRENT_SOURCE_DIR}/keybindings/default_keybindings.cfg
  )
  generate_translations("${translations_qrc}" "${ts_dir}" "${languages}" "${cfg_files}")
  list(APPEND qrc_files_cli ${translations_qrc})
endif()

set(qrc_files "${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc" ${translations_qrc})
set(icons_qrc_file "${CMAKE_CURRENT_BINARY_DIR}/icons/icons.qrc")
if (EXISTS ${icons_qrc_file})
  message(NOTICE "Found icons: ${icons_qrc_file}.")
  list(APPEND qrc_files ${icons_qrc_file})
else()
  message(NOTICE "Did not find icons at ${icons_qrc_file}.")
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

add_executable(ommpfritt "${compiled_resource_file}")
add_library(libommpfritt STATIC)

set(python_major_minor "python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}")
set(python_major "python${Python3_VERSION_MAJOR}")

set_warning_level(ommpfritt)
set_warning_level(libommpfritt)

if (BUILD_TRANSLATION)
  add_dependencies(resources translations_qm)
endif()
add_dependencies(libommpfritt registers)
add_dependencies(ommpfritt libommpfritt resources)

target_include_directories(libommpfritt PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${PROJECT_BINARY_DIR}"
)
target_compile_features(libommpfritt PUBLIC cxx_std_17)
target_link_libraries(libommpfritt Python3::Python)
target_link_libraries(libommpfritt Qt5::Widgets Qt5::Svg)
target_link_libraries(libommpfritt poppler-qt5)
target_link_libraries(libommpfritt -lpthread -lm)
target_link_libraries(libommpfritt KF5ItemModels)
target_link_libraries(libommpfritt 2Geom::2geom)
target_link_libraries(ommpfritt libommpfritt)

if (WIN32)
  target_link_libraries(libommpfritt -lopengl32)
else()
  target_link_libraries(libommpfritt -ldl -lutil)
  target_link_libraries(libommpfritt -lGL)
endif()

add_subdirectory(src)

if (BUILD_TESTING)
  include(CTest)
  add_subdirectory(test)
endif()

install(TARGETS ommpfritt)
# include(InstallRequiredSystemLibraries)
include(Packaging)

get_target_property(qt5_qmake_location Qt5::qmake IMPORTED_LOCATION)

if (WIN32)
  execute_process(COMMAND "${qt5_qmake_location}" -query QT_INSTALL_PREFIX
      RESULT_VARIABLE return_code
      OUTPUT_VARIABLE qt5_install_prefix
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  install(CODE "
    include(\"BundleUtilities\")
    execute_process(COMMAND
        \"${qt5_install_prefix}/bin/windeployqt.exe\"
        \"\${CMAKE_INSTALL_PREFIX}/bin/ommpfritt.exe\"
    )
    fixup_bundle(
      \"\${CMAKE_INSTALL_PREFIX}/bin/ommpfritt.exe\"
      \"\"
      \"\"
    )"
    COMPONENT Runtim
  )

  install(DIRECTORY ${Python3_STDLIB} TYPE LIB)
endif()

configure_file(${CMAKE_SOURCE_DIR}/cmake/generate-icons.py.in ${CMAKE_BINARY_DIR}/generate-icons.py @ONLY)
message("Run ${CMAKE_BINARY_DIR}/generate-icons.py to generate the icons, then build the project again
        to integrate the icons into the application.")
configure_file(${CMAKE_SOURCE_DIR}/cmake/make-appimage.sh.in ${CMAKE_BINARY_DIR}/make-appimage.sh @ONLY)
add_custom_target(appimage
                  COMMAND "${CMAKE_BINARY_DIR}/make-appimage.sh"
                  COMMENT "Packing AppImage"
)
