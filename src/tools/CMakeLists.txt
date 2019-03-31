file (GLOB SOURCES
  "brushselecttool.cpp"
  "knifetool.cpp"
  "positionvariant.cpp"
  "selecttool.cpp"
  "tool.cpp"
  "toolbox.cpp"
  "toolregister.cpp"
)

target_sources(libommpfritt PRIVATE ${SOURCES})

add_subdirectory(handles)
