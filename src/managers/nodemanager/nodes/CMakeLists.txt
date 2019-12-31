
file(GLOB SOURCES
  constantnode.cpp
  nodesregister.cpp
  mathnode.cpp
  referencenode.cpp
  spynode.cpp
)

file(GLOB HEADERS
  constantnode.h
  mathnode.h
  referencenode.h
  spynode.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

