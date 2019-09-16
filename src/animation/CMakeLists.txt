
file(GLOB SOURCES
  abstracttrack.cpp
  animator.cpp
  floattrack.cpp
  trackregister.cpp
)

file(GLOB HEADERS
  abstracttrack.h
  animator.h
  floattrack.h
  track.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

