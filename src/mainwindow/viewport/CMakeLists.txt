
file(GLOB SOURCES
  mousepancontroller.cpp
  viewport.cpp
)

file(GLOB HEADERS
  mousepancontroller.h
  viewport.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

