# Check whether the git version matches the cmake version.

execute_process(
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  COMMAND git describe --long
  OUTPUT_VARIABLE omm_git_version
  RESULT_VARIABLE gitdescribe_result
  )
string(REGEX MATCHALL "[0-9]+" omm_git_version "${omm_git_version}")

if ("${gitdescribe_result}" STREQUAL "0")
  list(GET omm_git_version 0 omm_git_version_major)
  list(GET omm_git_version 1 omm_git_version_minor)
  list(GET omm_git_version 2 omm_git_version_patch)
  set(omm_git_version "${omm_git_version_major}.${omm_git_version_minor}.${omm_git_version_patch}")

  if (NOT ${omm_git_version} STREQUAL "${CMAKE_PROJECT_VERSION}")
    message(FATAL_ERROR
      "Inconsistent version number: git: ${omm_git_version} but cmake: ${CMAKE_PROJECT_VERSION}"
    )
  endif()
else()
  # If git is not installed or the source code is not in a git repo, the build should not fail.
  message(WARNING "Failed to retrieve git version: ${gitdescribe_result}")
endif()
