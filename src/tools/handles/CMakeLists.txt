
file(GLOB SOURCES
  handle.cpp
  particlehandle.cpp
  selecthandle.cpp
  tangenthandle.cpp
)

file(GLOB HEADERS
  boundingboxhandle.h
  handle.h
  moveaxishandle.h
  particlehandle.h
  rotatehandle.h
  scalebandhandle.h
  selecthandle.h
  tangenthandle.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

