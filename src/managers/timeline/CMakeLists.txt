
file(GLOB SOURCES
  slider.cpp
  timeline.cpp
  timelinetitlebar.cpp
)

file(GLOB HEADERS
  slider.h
  timeline.h
  timelinetitlebar.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

