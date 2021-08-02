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

add_executable(ommpfritt-cli src/maincli.cpp "${compiled_resource_file}")
add_executable(ommpfritt src/main.cpp "${compiled_resource_file}")
add_library(libommpfritt STATIC)

set(python_major_minor "python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}")
set(python_major "python${Python3_VERSION_MAJOR}")
if (MINGW)
  get_filename_component(MinGWPath ${CMAKE_CXX_COMPILER} PATH)
  FILE (GLOB required_system_dlls
        "${MinGWPath}/libbrotlicommon.dll"
        "${MinGWPath}/libbrotlidec.dll"
        "${MinGWPath}/libbz2-1.dll"
        "${MinGWPath}/libcrypto-1_1-x64.dll"
        "${MinGWPath}/libcurl-4.dll"
        "${MinGWPath}/libdouble-conversion.dll"
        "${MinGWPath}/libfreetype-6.dll"
        "${MinGWPath}/libgcc_s_seh-1.dll"
        "${MinGWPath}/libglib-2.0-0.dll"
        "${MinGWPath}/libgraphite2.dll"
        "${MinGWPath}/libgsl-25.dll"
        "${MinGWPath}/libgslcblas-0.dll"
        "${MinGWPath}/libharfbuzz-0.dll"
        "${MinGWPath}/libiconv-2.dll"
        "${MinGWPath}/libicudt*.dll"
        "${MinGWPath}/libicuin*.dll"
        "${MinGWPath}/libicuuc*.dll"
        "${MinGWPath}/libidn2-0.dll"
        "${MinGWPath}/libintl-8.dll"
        "${MinGWPath}/libjpeg-8.dll"
        "${MinGWPath}/libKF5ItemModels.dll"
        "${MinGWPath}/liblcms2-2.dll"
        "${MinGWPath}/liblzma-5.dll"
        "${MinGWPath}/libnghttp2-14.dll"
        "${MinGWPath}/libnspr4.dll"
        "${MinGWPath}/libopenjp2-7.dll"
        "${MinGWPath}/libpcre-1.dll"
        "${MinGWPath}/libpcre2-16-0.dll"
        "${MinGWPath}/libplc4.dll"
        "${MinGWPath}/libplds4.dll"
        "${MinGWPath}/libpng16-16.dll"
        "${MinGWPath}/libpoppler-*.dll"
        "${MinGWPath}/libpoppler-qt5-1.dll"
        "${MinGWPath}/libpsl-5.dll"
        "${MinGWPath}/lib${python_major_minor}.dll"
        "${MinGWPath}/libssh2-1.dll"
        "${MinGWPath}/libssl-1_1-x64.dll"
        "${MinGWPath}/libstdc++-6.dll"
        "${MinGWPath}/libtiff-5.dll"
        "${MinGWPath}/libunistring-2.dll"
        "${MinGWPath}/libwinpthread-1.dll"
        "${MinGWPath}/libzstd.dll"
        "${MinGWPath}/nss3.dll"
        "${MinGWPath}/nssutil3.dll"
        "${MinGWPath}/Qt5Core.dll"
        "${MinGWPath}/Qt5Gui.dll"
        "${MinGWPath}/Qt5Svg.dll"
        "${MinGWPath}/Qt5Widgets.dll"
        "${MinGWPath}/Qt5WinExtras.dll"
        "${MinGWPath}/Qt5Xml.dll"
        "${MinGWPath}/smime3.dll"
        "${MinGWPath}/libdeflate.dll"
        "${MinGWPath}/libwebp-7.dll"
        "${MinGWPath}/zlib1.dll"
  )
  message(STATUS "mingw path: ${MinGWPath}")
  message(STATUS "dlls: ${required_system_dlls}")
  message(STATUS "python name: ${python_major_minor}")
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS ${required_system_dlls})
  install(FILES ${MinGWPath}/../share/qt5/plugins/platforms/qwindows.dll DESTINATION bin/platforms)

  file(GLOB python_files ${MinGWPath}/../lib/${python_major_minor}/*.py)
  foreach (fn IN LISTS python_files)
    install(FILES ${fn} DESTINATION lib/${python_major_minor}/)
  endforeach()

  install(FILES ${MinGWPath}/../lib/${python_major_minor}/LICENSE.txt DESTINATION lib/)
  list(APPEND python_dirs collections ctypes encodings json multiprocessing)
  foreach (dir IN LISTS python_dirs)
    install(DIRECTORY "${MinGWPath}/../lib/${python_major_minor}/${dir}" DESTINATION lib/${python_major_minor}/)
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
set_warning_level(libommpfritt)

if (BUILD_TRANSLATION)
  add_dependencies(resources translations_qm)
endif()
add_dependencies(libommpfritt registers)
add_dependencies(ommpfritt libommpfritt resources)
add_dependencies(ommpfritt-cli libommpfritt resources)

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

if (WIN32)
  target_link_libraries(libommpfritt -lopengl32)
else()
  target_link_libraries(libommpfritt -ldl -lutil)
  target_link_libraries(libommpfritt -lGL)
endif()

add_subdirectory(src)

if (BUILD_TESTING)
  add_executable(ommpfritt_unit_tests)
  set_warning_level(ommpfritt_unit_tests)
  add_dependencies(ommpfritt_unit_tests resources)
  target_link_libraries(ommpfritt_unit_tests libommpfritt)
  target_link_libraries(ommpfritt_unit_tests gtest)
  target_link_libraries(ommpfritt_unit_tests gtest_main)
  include(CTest)
  add_subdirectory(test)
endif()

install(
    TARGETS ommpfritt ommpfritt-cli
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static
)

SET(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}${SYSTEM_SUFFIX}")
include(CPack)
include(InstallRequiredSystemLibraries)

configure_file(${CMAKE_SOURCE_DIR}/cmake/generate-icons.py.in ${CMAKE_BINARY_DIR}/generate-icons.py @ONLY)
message("Run ${CMAKE_BINARY_DIR}/generate-icons.py to generate the icons, then build the project again
        to integrate the icons into the application.")
get_target_property(qt5_qmake_location Qt5::qmake IMPORTED_LOCATION)
configure_file(${CMAKE_SOURCE_DIR}/cmake/make-appimage.sh.in ${CMAKE_BINARY_DIR}/make-appimage.sh @ONLY)
add_custom_target(appimage
                  COMMAND "${CMAKE_BINARY_DIR}/make-appimage.sh"
                  COMMENT "Packing AppImage"
)
