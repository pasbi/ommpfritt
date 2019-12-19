
file(GLOB SOURCES
  constantnode.cpp
  nodesregister.cpp
  mathnode.cpp
  referencenode.cpp
)

file(GLOB HEADERS
  constantnode.h
  mathnode.h
  referencenode.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

