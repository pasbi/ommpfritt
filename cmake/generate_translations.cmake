function(generate_translations translations_qrc ts_dir languages prefixes)
    set(qm_dir "${CMAKE_BINARY_DIR}/qm/")

    file(MAKE_DIRECTORY ${qm_dir})
    set(script "${CMAKE_CURRENT_SOURCE_DIR}/build-scripts/generate-translations_qrc.py")

    add_custom_command(
        OUTPUT "${qm_dir}/translations.qrc"
        COMMAND_EXPAND_LISTS
        COMMAND ${PYTHON_EXECUTABLE} "${script}"
          --languages "${languages}"
          --prefixes "${prefixes}"
          --qrc "${translations_qrc}"
        COMMENT "generate translations.qrc"
    )

    add_custom_target(translations_qm
        DEPENDS "${translations_qrc}"
    )

    set_source_files_properties("${translations_qrc}" PROPERTIES SKIP_AUTORCC ON)

    foreach (lang IN LISTS languages)
        SET(qm "${qm_dir}/omm_${lang}.qm")
        SET(ts "${ts_dir}/omm_${lang}.ts")
        add_custom_command(
            OUTPUT
              "${qm}"
              "${qm_dir}/qt_${lang}.qm"
              "${qm_dir}/qtbase_${lang}.qm"
              "${qm_dir}/qt_help_${lang}.qm"
              "${qm_dir}/qtlocation_${lang}.qm"
            DEPENDS ${TS_SOURCES}
            COMMAND ${Qt5_LRELEASE_EXECUTABLE} ${ts} -qm ${qm}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${QT_QM_PATH}/qt_${lang}.qm" "${qm_dir}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${QT_QM_PATH}/qtbase_${lang}.qm" "${qm_dir}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${QT_QM_PATH}/qt_help_${lang}.qm" "${qm_dir}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${QT_QM_PATH}/qtlocation_${lang}.qm" "${qm_dir}"
            COMMENT "Prepare translation files '${lang}'"
        )
        add_custom_target("qm_${lang}"
            DEPENDS ${qm}
                    "${qm_dir}/qt_${lang}.qm"
                    "${qm_dir}/qtbase_${lang}.qm"
                    "${qm_dir}/qt_help_${lang}.qm"
                    "${qm_dir}/qtlocation_${lang}.qm"
        )
        add_dependencies(translations_qm "qm_${lang}")
    endforeach()
endfunction()

