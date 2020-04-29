function(generate_resources target compiled_resource_file qrc_files)
  add_custom_command(
      OUTPUT "${compiled_resource_file}"
      DEPENDS ${qrc_files}
      COMMAND_EXPAND_LISTS
      COMMAND Qt5::rcc
        -o "${compiled_resource_file}" ${qrc_files}
      COMMENT "Compile ${compiled_resource_file} from ${qrc_files}."
  )
  add_custom_target(${target} DEPENDS "${compiled_resource_file}")
endfunction()

