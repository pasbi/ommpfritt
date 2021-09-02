find_package(PkgConfig)
pkg_check_modules(PC_Poppler poppler)

find_path(Poppler_INCLUDE_DIR
    NAMES poppler-config.h
    HINTS ${PC_Poppler_INCLUDE_DIRS}
)

find_library(Poppler_LIBRARY
  NAMES poppler
  HINTS ${PC_Poppler_LIBRARY_DIRS}
)

set(Poppler_VERSION ${PC_Poppler_VERSION})
mark_as_advanced(Poppler_FOUND Poppler_INCLUDE_DIR Poppler_LIBRARY Poppler_VERSION)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Poppler
    REQUIRED_VARS Poppler_INCLUDE_DIR Poppler_LIBRARY
    VERSION_VAR Poppler_VERSION
)

message("defining targets ...")
if(Poppler_FOUND AND NOT TARGET Poppler::Poppler)
    add_library(Poppler::Core SHARED IMPORTED)
    set_target_properties(Poppler::Core PROPERTIES
        IMPORTED_LOCATION ${Poppler_LIBRARY}
    )
    target_include_directories(Poppler::Core INTERFACE ${Poppler_INCLUDE_DIR})
endif()
