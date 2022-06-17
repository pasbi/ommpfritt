cmake_minimum_required (VERSION 3.13)

# Suppresses a warning in cmake 3.x concerning a currently unused feature (setting VERSION in project() command)
if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

project(ommpfritt VERSION 0.1.4)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TRANSLATION "Build translations of the application." ON)
option(USE_QT_5_12 "Allow to use Qt 5.12. Set this option to true for static analyzing only.
                    Builds with this configuration are not supposed to be run."
                    OFF
)
option(DRAW_POINT_IDS "Draw the id of path points next to the point." OFF)
option(WERROR "Error on compiler warnings. Not available for MSVC." ON)

if (USE_QT_5_12)
  find_package(Qt5 5.12 REQUIRED COMPONENTS Widgets Svg LinguistTools)
else()
  find_package(Qt5 5.15 REQUIRED COMPONENTS Widgets Svg LinguistTools)
endif()

find_package(PkgConfig)
find_package(Python3 3.6 REQUIRED COMPONENTS Interpreter Development)
find_package(2Geom REQUIRED)
find_package(fmt 8.0.0 REQUIRED)
find_package(Boost 1.65 REQUIRED COMPONENTS graph)

set(python_major_minor "python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}")
set(python_major "python${Python3_VERSION_MAJOR}")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/" ${CMAKE_MODULE_PATH})
include(GetGitRevisionDescription)
git_describe(GIT_DESCRIBE "--tags")

add_executable(ommpfritt src/main/main.cpp)
set_target_properties(ommpfritt PROPERTIES WIN32_EXECUTABLE ON)
add_library(libommpfritt STATIC)
target_compile_definitions(libommpfritt PUBLIC QT_NO_KEYWORDS)
target_compile_definitions(ommpfritt PUBLIC QT_NO_KEYWORDS)

macro(set_warning_level target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /WX)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -pedantic)
    if (WERROR)
      target_compile_options(${target} PRIVATE -Werror)
    endif()
  endif()
endmacro()
set_warning_level(ommpfritt)
set_warning_level(libommpfritt)

set(generated_src "${CMAKE_BINARY_DIR}/generated")
set(script "${CMAKE_SOURCE_DIR}/build-scripts/generate-code.py")
file(MAKE_DIRECTORY "${generated_src}")
foreach(R tags managers nodes properties tools objects)
    SET(generated_cpp "${generated_src}/register_${R}.cpp")
    SET(spec_file "${CMAKE_SOURCE_DIR}/lists/${R}.lst")
    add_custom_command(
        OUTPUT ${generated_cpp}
        DEPENDS ${spec_file} ${script}
        COMMAND Python3::Interpreter ${script} ${spec_file} ${generated_cpp}
        COMMENT "Generating ${R} register."
        VERBATIM
    )
    target_sources(libommpfritt PRIVATE "${generated_cpp}")
endforeach()

set(cfg_files
  ${CMAKE_SOURCE_DIR}/uicolors/ui-colors-light.cfg
  ${CMAKE_SOURCE_DIR}/uicolors/ui-colors-dark.cfg
  ${CMAKE_SOURCE_DIR}/keybindings/default_keybindings.cfg
)
add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/translations.h"
    DEPENDS ${cfg_files}
    COMMAND_EXPAND_LISTS
    COMMAND Python3::Interpreter "${CMAKE_SOURCE_DIR}/build-scripts/update-translations_h.py"
      --input "${cfg_files}"
      --output "${CMAKE_BINARY_DIR}/translations.h"
)
target_sources(libommpfritt PRIVATE "${CMAKE_BINARY_DIR}/translations.h")

target_include_directories(libommpfritt PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
    "${PROJECT_BINARY_DIR}"
)
target_compile_features(libommpfritt PUBLIC cxx_std_20)
target_link_libraries(libommpfritt Python3::Python)
target_link_libraries(libommpfritt Qt5::Widgets Qt5::Svg)
target_link_libraries(libommpfritt poppler-qt5)
target_link_libraries(libommpfritt -lpthread -lm)
target_link_libraries(libommpfritt 2Geom::2geom)
target_link_libraries(libommpfritt fmt::fmt)
target_link_libraries(libommpfritt Boost::graph)
target_link_libraries(ommpfritt libommpfritt)

if (WIN32)
  target_link_libraries(libommpfritt -lopengl32)
else()
  target_link_libraries(libommpfritt -ldl -lutil)
  target_link_libraries(libommpfritt -lGL)
endif()

get_target_property(qt_qmake_location Qt5::qmake IMPORTED_LOCATION)
execute_process(COMMAND ${qt_qmake_location} -query QT_INSTALL_TRANSLATIONS
                OUTPUT_VARIABLE qt_qm_path
                OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_subdirectory(src)

# For some reason it insists in having that RUNTIME DESTINATION, although it uses cmake 3.16 and
# the documentation says it's not required.
# However, having it does not seem to change anything else, so there's no reason not to have it.
install(TARGETS ommpfritt RUNTIME DESTINATION bin)

if (WIN32)
  add_executable(ommpfritt-cli src/main/main.cpp)
  target_compile_definitions(ommpfritt-cli PUBLIC QT_NO_KEYWORDS)
  set_warning_level(ommpfritt-cli)
  target_link_libraries(ommpfritt-cli libommpfritt)
  install(TARGETS ommpfritt-cli RUNTIME DESTINATION bin)
endif()

include(generate_resources)
generate_resources()

if (BUILD_TESTING)
  include(CTest)
  add_subdirectory(test)
endif()

include(Packaging)

if (WIN32)
  set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
  include(InstallRequiredSystemLibraries)
  execute_process(COMMAND "${qt_qmake_location}" -query QT_INSTALL_PREFIX
      OUTPUT_VARIABLE qt_install_prefix
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  execute_process(COMMAND "${qt_qmake_location}" -query QT_INSTALL_PLUGINS
      OUTPUT_VARIABLE qt_install_plugins
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  install(DIRECTORY ${Python3_STDLIB} TYPE LIB)
  install(FILES "${qt_install_plugins}/platforms/qoffscreen.dll"
          DESTINATION "bin/platforms/")

  get_filename_component(MinGW_bin_path ${CMAKE_CXX_COMPILER} PATH)

  # These two dependencies are strange:
  # I don't know why this project depdends on them.
  # On my PC, fixup_bundle finds them and installs and packages them correctly.
  # On github's CI, however, these files are not installed/packaged, which results in a corrupted
  # artifact (the installed application will not be executable).
  install(FILES "${MinGW_bin_path}/libssl-1_1-x64.dll" "${MinGW_bin_path}/libcrypto-1_1-x64.dll"
          TYPE BIN)

  install(CODE "
    include(BundleUtilities)
    execute_process(COMMAND
        \"${qt_install_prefix}/bin/windeployqt.exe\"
        \"\${CMAKE_INSTALL_PREFIX}/bin/ommpfritt.exe\"
    )
    fixup_bundle(
      \"\${CMAKE_INSTALL_PREFIX}/bin/ommpfritt.exe\"
      \"\"
      \"\"
    )"
    COMPONENT Runtime
  )
endif()

configure_file(${CMAKE_SOURCE_DIR}/cmake/make-appimage.sh.in ${CMAKE_BINARY_DIR}/make-appimage.sh @ONLY)
add_custom_target(appimage
                  COMMAND "${CMAKE_BINARY_DIR}/make-appimage.sh"
                  COMMENT "Packing AppImage"
)
add_custom_target(icons
                  COMMAND Python3::Interpreter "${CMAKE_BINARY_DIR}/generate-icons.py"
                  COMMENT "Generating icons")

find_file(poppler_include "poppler/qt5/poppler-qt5.h")
if (NOT poppler_include)
    message(FATAL_ERROR "poppler/qt5/poppler-qt5.h not found.")
endif()
