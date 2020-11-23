function(generate_resources target compiled_resource_file qrc_files)
  add_custom_command(
    OUTPUT "${compiled_resource_file}"
    COMMAND Qt5::rcc -o "${compiled_resource_file}" ${qrc_files}
    COMMAND_EXPAND_LISTS
    DEPENDS translations_qm
            "${qrc_files}"
            "${CMAKE_CURRENT_SOURCE_DIR}/keybindings/default_keybindings.cfg"
            "${CMAKE_CURRENT_SOURCE_DIR}/layouts/default_layout.ini"
    COMMENT "Compile ${compiled_resource_file} from ${qrc_files}"
  )
  add_custom_target(${target} DEPENDS "${compiled_resource_file}")
endfunction()

