
file(GLOB SOURCES
  boundingbox.cpp
  cubic.cpp
  cubics.cpp
  matrix.cpp
  objecttransformation.cpp
  point.cpp
  polarcoordinates.cpp
  rectangle.cpp
  util.cpp
)

file(GLOB HEADERS
  boundingbox.h
  cubic.h
  cubics.h
  matrix.h
  objecttransformation.h
  point.h
  polarcoordinates.h
  rectangle.h
  util.h
  vec2.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

