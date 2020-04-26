function(generate_translations qrc_translations_cpp ts_dir languages)
    SET(qm_dir "${CMAKE_BINARY_DIR}/qm/")
    file(MAKE_DIRECTORY ${qm_dir})

    add_custom_command(
        OUTPUT "${qm_dir}/translations.qrc"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/translations.qrc"
                "${qm_dir}/translations.qrc"
        COMMENT "copy translations.qrc"
    )

    add_custom_target(translations_qm
        DEPENDS "${qm_dir}/translations.qrc"
    )

    set_source_files_properties("${qm_dir}/translations.qrc" PROPERTIES SKIP_AUTORCC ON)

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
            COMMENT "Update prepare translation files ${lang}"
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

    set_property(SOURCE ${qm_dir}/translations.qrc PROPERTY SKIP_AUTOGEN ON)
    set_property(SOURCE ${qrc_translations_cpp} PROPERTY SKIP_AUTOGEN ON)
    add_custom_command(
        OUTPUT ${qrc_translations_cpp}
        COMMAND Qt5::rcc
          "${qm_dir}/translations.qrc"
          -o ${qrc_translations_cpp}
        COMMENT "Pack translations"
    )
    add_custom_target(translations_qrc DEPENDS "${qrc_translations_cpp}")
endfunction()

