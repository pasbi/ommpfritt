function(generate_translations languages cfg_files dependencies translations_qrc)
    set(ts_dir "${CMAKE_SOURCE_DIR}/ts/")
    set(qm_dir "${CMAKE_BINARY_DIR}/qm")
    file(MAKE_DIRECTORY "${qm_dir}")
    if (WIN32)
      set(qt_translator_prefixes)
    else()
      set(qt_translator_prefixes "qt" "qtbase" "qt_help" "qtlocation")
    endif()


    file(MAKE_DIRECTORY ${qm_dir})
    list(TRANSFORM languages APPEND ".ts" OUTPUT_VARIABLE ts_files)
    list(TRANSFORM ts_files PREPEND "${ts_dir}/omm_")
    set(script "${CMAKE_SOURCE_DIR}/build-scripts/update-translations_h.py")
    set(translations_h "${CMAKE_SOURCE_DIR}/src/translations.h")

    file(GLOB_RECURSE TS_SOURCES
         CONFIGURE_DEPENDS
         "${CMAKE_SOURCE_DIR}/src/*.cpp"
         "${CMAKE_SOURCE_DIR}/src/*.h"
         "${CMAKE_SOURCE_DIR}/src/*.ui"
    )
    list(REMOVE_ITEM TS_SOURCES "${translations_h}")

    add_custom_command(
        OUTPUT "${translations_h}"
        DEPENDS ${cfg_files} "${script}"
          # ${TS_SOURCES}  # enabling this would be proper, but slow.
        COMMAND_EXPAND_LISTS
        COMMAND Python3::Interpreter "${script}"
          --input "${cfg_files}"
          --output "${translations_h}"
        COMMAND Qt5::lupdate
          -noobsolete
          ${CMAKE_SOURCE_DIR}/src
          -ts "${ts_files}"
        COMMENT "Update ts files. Check the files with `linguist` manually!"
    )
    target_sources(libommpfritt PRIVATE "${translations.h}")
    set(qm_files ${languages})
    list(TRANSFORM qm_files APPEND ".qm")
    list(TRANSFORM qm_files PREPEND "${qm_dir}/omm_")


    set(script "${CMAKE_SOURCE_DIR}/build-scripts/generate-translations_qrc.py")
    add_custom_command(
        OUTPUT "${translations_qrc}"
        DEPENDS "${qm_files}" "${translations.h}"
        COMMAND_EXPAND_LISTS
        COMMAND Python3::Interpreter "${script}"
          --languages "${languages}"
          --prefixes "omm" "${qt_translator_prefixes}"
          --qrc "${translations_qrc}"
        COMMENT "generate translations.qrc"
    )
    list(APPEND "${dependencies}" "${translations_qrc}")

    set_source_files_properties("${translations_qrc}" PROPERTIES SKIP_AUTORCC ON)

    get_target_property(qt5_qmake_location Qt5::qmake IMPORTED_LOCATION)
    execute_process(COMMAND ${qt5_qmake_location} -query QT_INSTALL_TRANSLATIONS
                    OUTPUT_VARIABLE QT_QM_PATH
                    ERROR_VARIABLE QT_QM_PATH_ERROR
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    foreach (lang IN LISTS languages)
        set(omm_qm "${qm_dir}/omm_${lang}.qm")
        set(omm_ts "${ts_dir}/omm_${lang}.ts")

        add_custom_command(
            OUTPUT "${omm_qm}"
            DEPENDS "${omm_ts}"
            COMMAND Qt5::lrelease ${omm_ts} -qm ${omm_qm}
            COMMENT "Prepare translation files '${lang}'"
        )
        list(APPEND "${dependencies}" "${omm_qm}")

        if (qt_translator_prefixes)
          set(qt_qm_files "${qt_translator_prefixes}")
          list(TRANSFORM qt_qm_files APPEND "_${lang}.qm")
          list(TRANSFORM qt_qm_files PREPEND "${qm_dir}/" OUTPUT_VARIABLE dst_qt_qm_files)
          list(TRANSFORM qt_qm_files PREPEND "${QT_QM_PATH}/" OUTPUT_VARIABLE src_qt_qm_files)
          add_custom_command(
              OUTPUT ${dst_qt_qm_files}
              COMMAND ${CMAKE_COMMAND} -E copy_if_different
                      ${src_qt_qm_files} "${qm_dir}"
              COMMENT "Copy Qt translation files '${lang}'"
          )
          list(APPEND "${dependencies}" ${dst_qt_qm_files})
        endif()
    endforeach()
    set("${dependencies}" "${${dependencies}}" PARENT_SCOPE)
endfunction()
