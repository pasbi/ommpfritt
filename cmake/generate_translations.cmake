function(generate_translations translations_qrc ts_dir languages cfg_files)
    set(qm_dir "${CMAKE_BINARY_DIR}/qm/")
    set(qt_translator_prefixes "qt" "qtbase" "qt_help" "qtlocation")

    file(MAKE_DIRECTORY ${qm_dir})
    list(TRANSFORM languages APPEND ".ts" OUTPUT_VARIABLE ts_files)
    list(TRANSFORM ts_files PREPEND "${ts_dir}/omm_")
    set(script "${CMAKE_CURRENT_SOURCE_DIR}/build-scripts/update-translations_h.py")
    set(translations_h "${CMAKE_CURRENT_SOURCE_DIR}/src/translations.h")

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
          ${CMAKE_CURRENT_SOURCE_DIR}/src
          -ts "${ts_files}"
        COMMENT "Update ts files. Check the files with `linguist` manually!"
    )
    add_custom_target(ts_target DEPENDS "${translations_h}" "${ts_files}")
    set(qm_files ${languages})
    list(TRANSFORM qm_files APPEND ".qm")
    list(TRANSFORM qm_files PREPEND "${qm_dir}/omm_")

    set(script "${CMAKE_CURRENT_SOURCE_DIR}/build-scripts/generate-translations_qrc.py")
    add_custom_command(
        OUTPUT "${qm_dir}/translations.qrc"
        DEPENDS ts_target ${qm_files}
        COMMAND_EXPAND_LISTS
        COMMAND Python3::Interpreter "${script}"
          --languages "${languages}"
          --prefixes "omm" "${qt_translator_prefixes}"
          --qrc "${translations_qrc}"
        COMMENT "generate translations.qrc"
    )

    add_custom_target(translations_qm DEPENDS "${translations_qrc}" )

    set_source_files_properties("${translations_qrc}" PROPERTIES SKIP_AUTORCC ON)

    get_target_property(qt5_qmake_location Qt5::qmake IMPORTED_LOCATION)
    execute_process(COMMAND ${qt5_qmake_location} -query QT_INSTALL_TRANSLATIONS
                    OUTPUT_VARIABLE QT_QM_PATH
                    ERROR_VARIABLE QT_QM_PATH_ERROR
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    foreach (lang IN LISTS languages)
        SET(qm "${qm_dir}/omm_${lang}.qm")
        SET(ts "${ts_dir}/omm_${lang}.ts")
        set(qt_qm_files "${qt_translator_prefixes}")
        list(TRANSFORM qt_qm_files APPEND "_${lang}.qm")
        list(TRANSFORM qt_qm_files PREPEND "${qm_dir}/" OUTPUT_VARIABLE dst_qt_qm_files)
        list(TRANSFORM qt_qm_files PREPEND "${QT_QM_PATH}/" OUTPUT_VARIABLE src_qt_qm_files)

        add_custom_command(
            OUTPUT "${qm}" ${dst_qt_qm_files}
            DEPENDS ${ts_files}
            COMMAND Qt5::lrelease ${ts} -qm ${qm}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${src_qt_qm_files} "${qm_dir}"
            COMMENT "Prepare translation files '${lang}'"
        )
        add_custom_target("qm_${lang}" DEPENDS ${qm} ${dst_qt_qm_files} )
        add_dependencies(translations_qm "qm_${lang}")
    endforeach()
endfunction()
