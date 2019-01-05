file (GLOB SOURCES
  "tool.cpp"
  "toolbox.cpp"
  "toolregister.cpp"
)
target_sources(libommpfritt PRIVATE ${SOURCES})

add_subdirectory(handles)
add_subdirectory(itemtools)
