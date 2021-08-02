function(generate_resources target compiled_resource_file qrc_files)
  if (BUILD_TRANSLATION)
    set(maybe_translation_qm translation_qm)
  else()
    unset(maybe_translation_qm)
  endif()
  add_custom_command(
    OUTPUT "${compiled_resource_file}"
    COMMAND Qt5::rcc -o "${compiled_resource_file}" ${qrc_files}
    COMMAND_EXPAND_LISTS
    DEPENDS ${maybe_translations_qm}
            "${qrc_files}"
            "${CMAKE_CURRENT_SOURCE_DIR}/keybindings/default_keybindings.cfg"
            "${CMAKE_CURRENT_SOURCE_DIR}/layouts/default_layout.ini"
    COMMENT "Compile ${compiled_resource_file} from ${qrc_files}"
  )
  add_custom_target(${target} DEPENDS "${compiled_resource_file}")
endfunction()

