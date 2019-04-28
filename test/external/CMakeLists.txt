set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(gtest)
set_target_properties(gtest PROPERTIES FOLDER "external")
set_target_properties(gtest_main PROPERTIES FOLDER "external")
