function(generate_icons icons_qrc classes)
  set(icons_dir "${CMAKE_CURRENT_BINARY_DIR}/icons")
  file(MAKE_DIRECTORY "${icons_dir}")

  set(specs "${classes}")
  list(TRANSFORM specs PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/lists/")
  list(TRANSFORM specs APPEND ".lst")

  set(script "${CMAKE_CURRENT_SOURCE_DIR}/build-scripts/generate-icons.py")
  set(scenefile "${CMAKE_CURRENT_SOURCE_DIR}/icons/icons.omm")

  add_custom_command(
    OUTPUT "${icons_qrc}"
    DEPENDS "${specs}" "${script}" "${scenefile}"
    COMMAND_EXPAND_LISTS
    COMMAND Python3::Interpreter "${script}"
      --specs ${specs}
      --command "${CMAKE_CURRENT_BINARY_DIR}/ommpfritt-cli"
      --scenefile "${scenefile}"
      --output "${icons_dir}"
      --qrc "${icons_qrc}"
    COMMENT "generate icons and qrc"
  )
  add_custom_target(icons_png "${icons_qrc}")
endfunction()

