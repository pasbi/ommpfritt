file (GLOB SOURCES
  "movetool.cpp"
  "scaletool.cpp"
  "objecttransformationtool.cpp"
  "rotatetool.cpp"
  "tool.cpp"
  "toolbox.cpp"
  "toolregister.cpp"
)
target_sources(libommpfritt PRIVATE ${SOURCES})

add_subdirectory(handles)
