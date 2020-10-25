function(generate_resources target compiled_resource_file qrc_files)
  add_custom_target(${target}
      COMMAND Qt5::rcc -o "${compiled_resource_file}" ${qrc_files}
      COMMAND_EXPAND_LISTS
      BYPRODUCTS "${compiled_resource_file}"
      DEPENDS ${qrc_files} ${source_files}
        "${CMAKE_CURRENT_SOURCE_DIR}/keybindings/default_keybindings.cfg"
        "${CMAKE_CURRENT_SOURCE_DIR}/layouts/default_layout.ini"
      COMMENT "Compile ${compiled_resource_file} from ${qrc_files}."
      )
  add_dependencies(${target} translations_qm)
endfunction()

