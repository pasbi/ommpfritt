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

set(python_major_minor "python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}")
set(python_major "python${Python3_VERSION_MAJOR}")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/" ${CMAKE_MODULE_PATH})
include(GetGitRevisionDescription)
git_describe(GIT_DESCRIBE "--tags")

add_executable(ommpfritt)
set_target_properties(ommpfritt PROPERTIES WIN32_EXECUTABLE ON)
add_library(libommpfritt STATIC)
target_compile_definitions(libommpfritt PUBLIC QT_NO_KEYWORDS)
target_compile_definitions(ommpfritt PUBLIC QT_NO_KEYWORDS)

macro(set_warning_level target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /WX)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -pedantic -Werror)
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
set(rcc_dependencies ${cfg_files} "${CMAKE_SOURCE_DIR}/layouts/default_layout.ini")
set(qrc_files "${CMAKE_SOURCE_DIR}/resources.qrc")
if (BUILD_TRANSLATION)
  include(generate_translations)
  set(qrc_translations_cpp "${CMAKE_BINARY_DIR}/qrc_translations.cpp")
  set(languages de en es)
  set(translations_qrc "${CMAKE_BINARY_DIR}/qm/translations.qrc")
  generate_translations("${languages}" "${cfg_files}" rcc_dependencies "${translations_qrc}")
  list(APPEND qrc_files "${translations_qrc}")
endif()

set(icons_qrc_file "${CMAKE_BINARY_DIR}/icons/icons.qrc")
configure_file(${CMAKE_SOURCE_DIR}/cmake/generate-icons.py.in ${CMAKE_BINARY_DIR}/generate-icons.py @ONLY)
if (EXISTS ${icons_qrc_file})
  message(NOTICE "Found icons: ${icons_qrc_file}.")
  list(APPEND qrc_files ${icons_qrc_file})
  if (WIN32)
    configure_file(${CMAKE_SOURCE_DIR}/icons/resource.rc.in ${CMAKE_BINARY_DIR}/resource.rc COPYONLY)
    target_sources(ommpfritt PRIVATE "${CMAKE_BINARY_DIR}/resource.rc")
  endif()
else()
  message(NOTICE "Did not find icons at ${icons_qrc_file}.")
  message("Run ${CMAKE_BINARY_DIR}/generate-icons.py to generate the icons, then build the project again
          to integrate the icons into the application.")
endif()

set(compiled_resource_file ${CMAKE_BINARY_DIR}/qrc_resources.cpp)
add_custom_command(
  OUTPUT "${compiled_resource_file}"
  COMMAND Qt5::rcc -o "${compiled_resource_file}" ${qrc_files}
  COMMAND_EXPAND_LISTS
  DEPENDS ${qrc_files} ${rcc_dependencies}
  COMMENT "Compile ${compiled_resource_file} from ${qrc_files}"
)
target_sources(ommpfritt PRIVATE "${compiled_resource_file}")

target_include_directories(libommpfritt PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
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

install(TARGETS ommpfritt RUNTIME DESTINATION bin)
include(Packaging)

get_target_property(qt5_qmake_location Qt5::qmake IMPORTED_LOCATION)

if (WIN32)
  set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
  include(InstallRequiredSystemLibraries)
  execute_process(COMMAND "${qt5_qmake_location}" -query QT_INSTALL_PREFIX
      OUTPUT_VARIABLE qt5_install_prefix
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  execute_process(COMMAND "${qt5_qmake_location}" -query QT_INSTALL_PLUGINS
      OUTPUT_VARIABLE qt5_install_plugins
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  install(DIRECTORY ${Python3_STDLIB} TYPE LIB)
  install(FILES "${qt5_install_plugins}/platforms/qoffscreen.dll"
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
        \"${qt5_install_prefix}/bin/windeployqt.exe\"
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

install(FILES "${CMAKE_SOURCE_DIR}/LICENSE" DESTINATION "." RENAME LICENSE.txt)
