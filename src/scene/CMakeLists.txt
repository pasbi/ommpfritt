file (GLOB SOURCES
  "abstractselectionobserver.cpp"
  "list.cpp"
  "scene.cpp"
  "structure.cpp"
  "tree.cpp"
  "propertyownermimedata.cpp"
  "itemmodeladapter.cpp"
  "objecttreeadapter.cpp"
  "listadapter.cpp"
)

add_subdirectory(history)

target_sources(libommpfritt PRIVATE ${SOURCES})

