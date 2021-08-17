function(generate_resources target)
  set(cfg_files
    ${CMAKE_SOURCE_DIR}/uicolors/ui-colors-light.cfg
    ${CMAKE_SOURCE_DIR}/uicolors/ui-colors-dark.cfg
    ${CMAKE_SOURCE_DIR}/keybindings/default_keybindings.cfg
  )
  set(rcc_dependencies ${cfg_files} "${CMAKE_SOURCE_DIR}/layouts/default_layout.ini")
  set(qrc_files "${CMAKE_SOURCE_DIR}/resources.qrc")
  if (BUILD_TRANSLATION)
    include(generate_translations)
    set(qrc_translations_cpp "${CMAKE_BINARY_DIR}/qrc_translations.cpp")
    set(languages de en es)
    set(translations_qrc "${CMAKE_BINARY_DIR}/qm/translations.qrc")
    generate_translations("${languages}" "${cfg_files}" rcc_dependencies "${translations_qrc}")
    list(APPEND qrc_files "${translations_qrc}")
  endif()

  set(icons_qrc_file "${CMAKE_BINARY_DIR}/icons/icons.qrc")
  configure_file(${CMAKE_SOURCE_DIR}/cmake/generate-icons.py.in ${CMAKE_BINARY_DIR}/generate-icons.py @ONLY)
  if (EXISTS ${icons_qrc_file})
    message(NOTICE "Found icons: ${icons_qrc_file}.")
    list(APPEND qrc_files ${icons_qrc_file})
    if (WIN32)
      configure_file(${CMAKE_SOURCE_DIR}/icons/resource.rc.in ${CMAKE_BINARY_DIR}/resource.rc COPYONLY)
      target_sources(${target} PRIVATE "${CMAKE_BINARY_DIR}/resource.rc")
    endif()
  else()
    message(NOTICE "Did not find icons at ${icons_qrc_file}.")
    message("Run ${CMAKE_BINARY_DIR}/generate-icons.py to generate the icons, then build the project again
            to integrate the icons into the application.")
  endif()

  set(compiled_resource_file ${CMAKE_BINARY_DIR}/qrc_resources.cpp)
  message("qrc files: ${qrc_files}")
  add_custom_command(
    OUTPUT "${compiled_resource_file}"
    COMMAND Qt5::rcc -o "${compiled_resource_file}" ${qrc_files}
    COMMAND_EXPAND_LISTS
    DEPENDS ${qrc_files} ${rcc_dependencies}
    COMMENT "Compile ${compiled_resource_file} from ${qrc_files}"
  )
  target_sources(${target} PRIVATE "${compiled_resource_file}")
endfunction()
