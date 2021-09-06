function(copy_qt_qm_files name languages)
  set(source_files ${languages})
  list(TRANSFORM source_files APPEND ".qm")
  list(TRANSFORM source_files PREPEND "${qt_qm_path}/${name}_")

  set(target_dir "${CMAKE_BINARY_DIR}/")

  set(target_files ${languages})
  list(TRANSFORM target_files APPEND ".qm")
  list(TRANSFORM target_files PREPEND "${target_dir}/${name}_")
  file(MAKE_DIRECTORY ${target_dir})
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${source_files} ${target_dir})
  list(APPEND qm_files ${target_files})
  set(qm_files ${qm_files} PARENT_SCOPE)
endfunction()

function(create_omm_qm_files languages)
  set(ts_files ${languages})
  list(TRANSFORM ts_files APPEND ".ts")
  list(TRANSFORM ts_files PREPEND "${CMAKE_SOURCE_DIR}/ts/omm_")
  qt_create_translation(omm_qm_files
                        ${CMAKE_SOURCE_DIR}/src "${CMAKE_BINARY_DIR}/translations.h"
                        ${ts_files}
                        OPTIONS -no-obsolete
  )
  list(APPEND qm_files ${omm_qm_files})
  set(qm_files ${qm_files} PARENT_SCOPE)
endfunction()

function(generate_resources)
  set(cfg_files
    ${CMAKE_SOURCE_DIR}/uicolors/ui-colors-light.cfg
    ${CMAKE_SOURCE_DIR}/uicolors/ui-colors-dark.cfg
    ${CMAKE_SOURCE_DIR}/keybindings/default_keybindings.cfg
  )
  set(rcc_dependencies ${cfg_files} "${CMAKE_SOURCE_DIR}/layouts/default_layout.ini")
  set(qrc_files "${CMAKE_SOURCE_DIR}/resources.qrc")
  if (BUILD_TRANSLATION)
    set(languages de en es)
    unset(qm_files)
    copy_qt_qm_files("qtbase" "${languages}")
    create_omm_qm_files("${languages}")

    target_sources(libommpfritt PRIVATE ${qm_files})
    foreach (qm_file ${qm_files})
      get_filename_component(qm_filename ${qm_file} NAME)
      set(QM_FILES_N "${QM_FILES_N}<file>${qm_filename}</file>\n")
    endforeach()
    file(WRITE
         "${CMAKE_BINARY_DIR}/translations.qrc"
         "<RCC version=\"1.0\">\n"
         "    <qresource prefix=\"/qm\">\n"
         "${QM_FILES_N}"
         "    </qresource>\n"
         "</RCC>\n"
    )
    list(APPEND qrc_files "${CMAKE_BINARY_DIR}/translations.qrc")
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
    COMMAND Qt::rcc -o "${compiled_resource_file}" ${qrc_files}
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
