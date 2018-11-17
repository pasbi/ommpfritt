file (GLOB SOURCES
  "manager.cpp"
)

target_sources(libommpfritt PRIVATE ${SOURCES})

add_subdirectory("objectmanager")
add_subdirectory("propertymanager")

