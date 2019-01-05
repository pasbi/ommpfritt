file (GLOB SOURCES
  "transformationtool.cpp"
  "movetool.cpp"
  "positionvariant.cpp"
  "rotatetool.cpp"
  "selecttool.cpp"
  "scaletool.cpp"
)

target_sources(libommpfritt PRIVATE ${SOURCES})
