
file(GLOB SOURCES
  slider.cpp
  timeline.cpp
  timelinecanvas.cpp
  timelinetitlebar.cpp
)

file(GLOB HEADERS
  slider.h
  timeline.h
  timelinecanvas.h
  timelinetitlebar.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

