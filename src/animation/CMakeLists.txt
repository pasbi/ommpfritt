
file(GLOB SOURCES
  animator.cpp
  track.cpp
  trackregister.cpp
)

file(GLOB HEADERS
  animator.h
  track.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

