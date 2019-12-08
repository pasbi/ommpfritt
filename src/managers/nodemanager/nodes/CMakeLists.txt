
file(GLOB SOURCES
  gradientnode.cpp
  nodesregister.cpp
)

file(GLOB HEADERS
  gradientnode.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

