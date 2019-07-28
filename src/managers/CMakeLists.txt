file (GLOB SOURCES
  "itemmanager.cpp"
  "manager.cpp"
  "manageritemview.cpp"
  "managerregister.cpp"
)

target_sources(libommpfritt PRIVATE ${SOURCES})

add_subdirectory("boundingboxmanager")
add_subdirectory("historymanager")
add_subdirectory("objectmanager")
add_subdirectory("propertymanager")
add_subdirectory("pythonconsole")
add_subdirectory("stylemanager")
