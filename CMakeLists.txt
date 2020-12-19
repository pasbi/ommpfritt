cmake_minimum_required (VERSION 3.13)

# Suppresses a warning in cmake 3.x concerning a currently unused feature (setting VERSION in project() command)
if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

project(ommpfritt VERSION 0.1.2)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(GENERATE_ICONS "There will be no icons in the application if this option is disabled." ON)
option(BUILD_TESTS "build the unit tests." ON)

find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5 COMPONENTS LinguistTools)
find_package(PkgConfig)
find_package(Python3 3.7 REQUIRED COMPONENTS Interpreter Development)
find_package(KF5ItemModels REQUIRED)
find_package(2Geom REQUIRED)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/" ${CMAKE_MODULE_PATH})
include(check_version_consistency)
include(generate_registers)
include(generate_translations)
include(generate_icons)
include(generate_resources)

list(APPEND classes tags managers nodes properties tools objects)
generate_registers("${classes}")

set (QT_QM_PATH CACHE PATH
    "path where to find the Qt translation files (qt_en.qm, qtbase_de.qm, etc.")

include(update_git_describe)

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
  set(specs "${classes};extra")
  list(TRANSFORM specs PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/lists/")
  list(TRANSFORM specs APPEND ".lst")
  set(specs "${specs};${CMAKE_CURRENT_SOURCE_DIR}/keybindings/default_keybindings.cfg")
  generate_icons("${icons_qrc}" "${specs}")
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

if (MINGW)
  get_filename_component(MinGWPath ${CMAKE_CXX_COMPILER} PATH)
  SET(required_system_dlls
        libbrotlicommon.dll
        libbrotlidec.dll
        libbz2-1.dll
        libcrypto-1_1-x64.dll
        libcurl-4.dll
        libdouble-conversion.dll
        libfreetype-6.dll
        libgcc_s_seh-1.dll
        libglib-2.0-0.dll
        libgraphite2.dll
        libgsl-25.dll
        libgslcblas-0.dll
        libharfbuzz-0.dll
        libiconv-2.dll
        libicudt67.dll
        libicuin67.dll
        libicuuc67.dll
        libidn2-0.dll
        libintl-8.dll
        libjpeg-8.dll
        libKF5ItemModels.dll
        liblcms2-2.dll
        liblzma-5.dll
        libnghttp2-14.dll
        libnspr4.dll
        libopenjp2-7.dll
        libpcre-1.dll
        libpcre2-16-0.dll
        libplc4.dll
        libplds4.dll
        libpng16-16.dll
        libpoppler-103.dll
        libpoppler-qt5-1.dll
        libpsl-5.dll
        libpython3.8.dll
        libssh2-1.dll
        libssl-1_1-x64.dll
        libstdc++-6.dll
        libtiff-5.dll
        libunistring-2.dll
        libwinpthread-1.dll
        libzstd.dll
        nss3.dll
        nssutil3.dll
        Qt5Core.dll
        Qt5Gui.dll
        Qt5Svg.dll
        Qt5Widgets.dll
        Qt5WinExtras.dll
        Qt5Xml.dll
        smime3.dll
        zlib1.dll
  )
  message(STATUS "mingw path: ${MinGWPath}")
  list(TRANSFORM required_system_dlls PREPEND "${MinGWPath}/")
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS ${required_system_dlls})
  install(FILES ${MinGWPath}/../share/qt5/plugins/platforms/qwindows.dll DESTINATION bin/platforms)

  file(GLOB python_filefwefwrs ${MinGWPath}/../lib/python3.8/*.py)
  foreach (fn IN LISTS python_filefwefwrs)
    install(FILES ${fn} DESTINATION lib/python3.8/)
  endforeach()

  install(FILES ${MinGWPath}/../lib/python3.8/LICENSE.txt DESTINATION lib/)
  list(APPEND python_dirsfwfrw collections ctypes encodings json multiprocessing)
  foreach (dir IN LISTS python_dirsfwfrw)
    install(DIRECTORY "${MinGWPath}/../lib/python3.8/${dir}" DESTINATION lib/python3.8/)
  endforeach()

  add_custom_command(TARGET ommpfritt-cli POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/platforms"
          COMMAND ${CMAKE_COMMAND} -E copy "${MinGWPath}/../share/qt5/plugins/platforms/qwindows.dll" "${PROJECT_BINARY_DIR}/platforms/"
          COMMAND ${CMAKE_COMMAND} -E copy ${required_system_dlls} ${PROJECT_BINARY_DIR}
          COMMAND_EXPAND_LISTS
          COMMENT "Copying dependencies into build directory."
          )
endif()

set_warning_level(ommpfritt-cli)
set_warning_level(ommpfritt)
set_warning_level(ommpfritt_unit_tests)
set_warning_level(libommpfritt)

if (GENERATE_ICONS)
  add_dependencies(icons_png ommpfritt-cli)
endif()

add_dependencies(resources_cli translations_qm)
add_dependencies(resources translations_qm)
add_dependencies(libommpfritt registers)
add_dependencies(libommpfritt update-git-describe)
add_dependencies(ommpfritt libommpfritt resources)
add_dependencies(ommpfritt_unit_tests resources_cli)
add_dependencies(ommpfritt-cli libommpfritt resources_cli)

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
    TARGETS ommpfritt ommpfritt-cli
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static
)

SET(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}${SYSTEM_SUFFIX}")
include(CPack)
include(InstallRequiredSystemLibraries)

