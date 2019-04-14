file (GLOB SOURCES
  "boundingbox.cpp"
  "cubic.cpp"
  "cubics.cpp"
  "edge.cpp"
  "matrix.cpp"
  "objecttransformation.cpp"
  "point.cpp"
  "polarcoordinates.cpp"
  "polygon.cpp"
  "rectangle.cpp"
  "triangle.cpp"
  "triangulation.cpp"
  "util.cpp"
)
target_sources(libommpfritt PRIVATE ${SOURCES})
