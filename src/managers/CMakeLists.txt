file (GLOB SOURCES
  "manager.cpp"
  "propertymanager.cpp"
)

target_sources(ommpfritt PRIVATE ${SOURCES})

add_subdirectory("objectmanager")

