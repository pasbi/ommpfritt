
file(GLOB SOURCES
  slider.cpp
  timeline.cpp
)

file(GLOB HEADERS
  slider.h
  timeline.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

