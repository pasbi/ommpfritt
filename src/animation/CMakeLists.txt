
file(GLOB SOURCES
  animator.cpp
  channelproxy.cpp
  track.cpp
  trackregister.cpp
)

file(GLOB HEADERS
  animator.h
  channelproxy.h
  track.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

