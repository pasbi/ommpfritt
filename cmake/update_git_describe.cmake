message("in cmake update git describe")

set(git_describe_fn "${CMAKE_BINARY_DIR}/git_describe.h")
add_custom_command(
  OUTPUT git_describe.h
  COMMAND ${CMAKE_SOURCE_DIR}/build-scripts/generate-git-describe.sh "${git_describe_fn}"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  DEPENDS .git/index
  COMMENT "Update git describe"
)
add_custom_target(update-git-describe DEPENDS "${git_describe_fn}")
