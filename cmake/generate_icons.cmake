function(generate_icons icons_qrc specs)
  set(icons_dir "${CMAKE_CURRENT_BINARY_DIR}/icons")
  file(MAKE_DIRECTORY "${icons_dir}")

  set(script "${CMAKE_CURRENT_SOURCE_DIR}/build-scripts/generate-icons.py")
  set(scenefile "${CMAKE_CURRENT_SOURCE_DIR}/icons/icons.omm")
  set(canned_icons
    ${CMAKE_CURRENT_SOURCE_DIR}/icons/BrushSelectTool.png
    ${CMAKE_CURRENT_SOURCE_DIR}/icons/KnifeTool.png
    ${CMAKE_CURRENT_SOURCE_DIR}/icons/PathTool.png
    ${CMAKE_CURRENT_SOURCE_DIR}/icons/ScriptTag.png
    ${CMAKE_CURRENT_SOURCE_DIR}/icons/SelectObjectsTool.png
    ${CMAKE_CURRENT_SOURCE_DIR}/icons/SelectPointsTool.png
  )

  add_custom_command(
    OUTPUT "${icons_qrc}"
    DEPENDS ommpfritt-cli "${specs}" "${script}" "${scenefile}"
    COMMAND_EXPAND_LISTS
    COMMAND Python3::Interpreter "${script}"
      --specs ${specs}
      --command "${CMAKE_CURRENT_BINARY_DIR}/ommpfritt-cli"
      --scenefile "${scenefile}"
      --canned ${canned_icons}
      --output "${icons_dir}"
      --qrc "${icons_qrc}"
    COMMENT "generate icons and qrc"
  )
  add_custom_target(icons_png "${icons_qrc}")
endfunction()

