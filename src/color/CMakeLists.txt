
file(GLOB SOURCES
  namedcolors.cpp
  color.cpp
)

file(GLOB HEADERS
  namedcolors.h
  color.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

