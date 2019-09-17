
file(GLOB SOURCES
  animator.cpp
  floattrack.cpp
  track.cpp
  trackregister.cpp
)

file(GLOB HEADERS
  animator.h
  floattrack.h
  track.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

