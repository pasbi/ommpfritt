file (GLOB SOURCES
  "manager.cpp"
)

target_sources(ommpfritt PRIVATE ${SOURCES})

add_subdirectory("objectmanager")
add_subdirectory("propertymanager")

