
file(GLOB SOURCES
  anchorhud.cpp
  mousepancontroller.cpp
  viewport.cpp
)

file(GLOB HEADERS
  anchorhud.h
  headupdisplay.h
  mousepancontroller.h
  viewport.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

