function(generate_resources)
  set(cfg_files
    ${CMAKE_SOURCE_DIR}/uicolors/ui-colors-light.cfg
    ${CMAKE_SOURCE_DIR}/uicolors/ui-colors-dark.cfg
    ${CMAKE_SOURCE_DIR}/keybindings/default_keybindings.cfg
  )
  set(rcc_dependencies ${cfg_files} "${CMAKE_SOURCE_DIR}/layouts/default_layout.ini")
  set(qrc_files "${CMAKE_SOURCE_DIR}/resources.qrc")
  if (BUILD_TRANSLATION)
    set(ts_files ${CMAKE_SOURCE_DIR}/ts/omm_de.ts
                 ${CMAKE_SOURCE_DIR}/ts/omm_en.ts
                 ${CMAKE_SOURCE_DIR}/ts/omm_es.ts
    )
    qt5_create_translation(QM_FILES
                           ${CMAKE_SOURCE_DIR}/src "${CMAKE_BINARY_DIR}/translations.h"
                           ${ts_files}
                           OPTIONS -no-obsolete
    )
    target_sources(libommpfritt PRIVATE ${QM_FILES})
    install(FILES ${QM_FILES} DESTINATION "${CMAKE_INSTALL_PREFIX}/qm")
    add_custom_target(update-ts DEPENDS ${ts_files})
  endif()

  set(icons_qrc_file "${CMAKE_BINARY_DIR}/icons/icons.qrc")
  configure_file(${CMAKE_SOURCE_DIR}/cmake/generate-icons.py.in ${CMAKE_BINARY_DIR}/generate-icons.py @ONLY)
  if (EXISTS ${icons_qrc_file})
    message(NOTICE "Found icons: ${icons_qrc_file}.")
    list(APPEND qrc_files ${icons_qrc_file})
    if (WIN32)
      configure_file(${CMAKE_SOURCE_DIR}/icons/resource.rc.in ${CMAKE_BINARY_DIR}/resource.rc COPYONLY)
      target_sources(ommpfritt PRIVATE "${CMAKE_BINARY_DIR}/resource.rc")
    endif()
  else()
    message(NOTICE "Did not find icons at ${icons_qrc_file}.")
    message("Run `${CMAKE_BINARY_DIR}/generate-icons.py` or build the target `icons` to generate
            the icons, then configure and build the project again to integrate the icons into the
            application.")
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
  target_sources(ommpfritt PRIVATE "${compiled_resource_file}")
  if (TARGET ommpfritt-cli)
    target_sources(ommpfritt-cli PRIVATE "${compiled_resource_file}")
  endif()
  set(compiled_resource_file "${compiled_resource_file}" PARENT_SCOPE)
endfunction()
