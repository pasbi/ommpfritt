file (GLOB SOURCES
  "manager.cpp"
  "managerregister.cpp"
)

target_sources(libommpfritt PRIVATE ${SOURCES})

add_subdirectory("objectmanager")
add_subdirectory("propertymanager")
add_subdirectory("stylemanager")

