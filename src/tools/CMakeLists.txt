file (GLOB SOURCES
  "movetool.cpp"
  "tool.cpp"
  "toolbox.cpp"
  "toolregister.cpp"
)
target_sources(libommpfritt PRIVATE ${SOURCES})

add_subdirectory(handles)
