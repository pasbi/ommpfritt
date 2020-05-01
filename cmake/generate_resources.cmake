function(generate_resources target compiled_resource_file qrc_files)
  add_custom_command(
      OUTPUT "${compiled_resource_file}"
      DEPENDS ${qrc_files} ${source_files}
        "${CMAKE_CURRENT_SOURCE_DIR}/keybindings/default_keybindings.cfg"
      COMMAND_EXPAND_LISTS
      COMMAND Qt5::rcc
        -o "${compiled_resource_file}" ${qrc_files}
      COMMENT "Compile ${compiled_resource_file} from ${qrc_files}."
  )
  add_custom_target(${target} DEPENDS "${compiled_resource_file}")
  add_dependencies(${target} translations_qm)
endfunction()

