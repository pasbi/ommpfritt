find_package(PkgConfig)
pkg_check_modules(PC_Poppler-Qt6 poppler-qt6)

find_path(Poppler-Qt6_INCLUDE_DIR
    NAMES poppler-qt6.h
    HINTS ${PC_Poppler-Qt6_INCLUDE_DIRS}
)

find_library(Poppler-Qt6_LIBRARY
  NAMES poppler-qt6
  HINTS ${PC_Poppler-Qt6_LIBRARY_DIRS}
)

set(Poppler-Qt6_VERSION ${PC_Poppler-Qt6_VERSION})
mark_as_advanced(Poppler-Qt6_FOUND Poppler-Qt6_INCLUDE_DIR Poppler-Qt6_LIBRARY Poppler-Qt6_VERSION)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Poppler-Qt6
    REQUIRED_VARS Poppler-Qt6_INCLUDE_DIR Poppler-Qt6_LIBRARY
    VERSION_VAR Poppler-Qt6_VERSION
)

if(Poppler-Qt6_FOUND AND NOT TARGET Poppler::Qt6)
    add_library(Poppler::Qt6 SHARED IMPORTED)
    set_target_properties(Poppler::Qt6 PROPERTIES
        IMPORTED_LOCATION ${Poppler-Qt6_LIBRARY}
    )
    target_include_directories(Poppler::Qt6 INTERFACE ${Poppler-Qt6_INCLUDE_DIR})
endif()
