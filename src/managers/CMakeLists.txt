file (GLOB SOURCES
  "itemmanager.cpp"
  "manager.cpp"
  "manageritemview.cpp"
  "managerregister.cpp"
)

target_sources(libommpfritt PRIVATE ${SOURCES})

add_subdirectory("objectmanager")
add_subdirectory("propertymanager")
add_subdirectory("stylemanager")
