
file(GLOB SOURCES
  boundingboxmanager.cpp
)

file(GLOB HEADERS
  boundingboxmanager.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

