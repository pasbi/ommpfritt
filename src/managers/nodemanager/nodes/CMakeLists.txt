
file(GLOB SOURCES
  constantnode.cpp
  composenode.cpp
  decomposenode.cpp
  nodesregister.cpp
  mathnode.cpp
  referencenode.cpp
  spynode.cpp
)

file(GLOB HEADERS
  constantnode.h
  composenode.h
  decomposenode.h
  mathnode.h
  referencenode.h
  spynode.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

