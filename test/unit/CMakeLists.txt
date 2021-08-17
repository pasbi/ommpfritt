include(GoogleTest)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/testconfig.h @ONLY)

macro(package_add_test SOURCE_FILE)
    string(REGEX REPLACE "\.[^.]*$" "" TESTNAME "${SOURCE_FILE}")
    set_source_files_properties("${compiled_resource_file}" TARGET_DIRECTORY libommpfritt PROPERTIES GENERATED ON)
    add_executable(${TESTNAME} main.cpp ${SOURCE_FILE} ${compiled_resource_file})
    target_link_libraries(${TESTNAME} PRIVATE gtest gmock libommpfritt)
    target_compile_definitions(${TESTNAME} PRIVATE "PYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}")
    target_include_directories(${TESTNAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

    if(BUILD_SHARED_LIBS)
      target_compile_definitions(${TESTNAME} PRIVATE "GTEST_LINKED_AS_SHARED_LIBRARY=1")
    endif()

    if (WIN32)
        add_dependencies(${TESTNAME} copy_dlls)
    endif()

    gtest_discover_tests(${TESTNAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
endmacro()

package_add_test(color.cpp)
package_add_test(common.cpp)
package_add_test(dnftest.cpp)
package_add_test(geometry.cpp)
package_add_test(pathtest.cpp)
package_add_test(propertytest.cpp)
package_add_test(splinetypetest.cpp)
package_add_test(stringinterpolation.cpp)
package_add_test(tree.cpp)
package_add_test(iotest.cpp)
