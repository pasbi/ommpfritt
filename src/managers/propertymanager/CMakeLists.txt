file (GLOB SOURCES
  "propertymanager.cpp"
  "propertymanagertab.cpp"
)

target_sources(libommpfritt PRIVATE ${SOURCES})
add_subdirectory("userpropertymanager")

