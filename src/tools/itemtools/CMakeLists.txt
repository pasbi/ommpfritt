file (GLOB SOURCES
  "multitransformationtool.cpp"
  "movetool.cpp"
  "positionvariant.cpp"
  "rotatetool.cpp"
  "selecttool.cpp"
  "scaletool.cpp"
  "transformationtool.cpp"
)

target_sources(libommpfritt PRIVATE ${SOURCES})
