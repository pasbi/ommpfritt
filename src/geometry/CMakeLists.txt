file (GLOB SOURCES
  "boundingbox.cpp"
  "cubic.cpp"
  "cubics.cpp"
  "matrix.cpp"
  "objecttransformation.cpp"
  "point.cpp"
  "polarcoordinates.cpp"
  "rectangle.cpp"
  "util.cpp"
)
target_sources(libommpfritt PRIVATE ${SOURCES})
