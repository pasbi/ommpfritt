
file(GLOB SOURCES
  color.cpp
)

file(GLOB HEADERS
  color.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

