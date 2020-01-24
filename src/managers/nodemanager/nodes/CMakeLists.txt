
file(GLOB SOURCES
  constantnode.cpp
  composenode.cpp
  decomposenode.cpp
  decomposecolornode.cpp
  composecolornode.cpp
  fragmentnode.cpp
  nodesregister.cpp
  mathnode.cpp
  referencenode.cpp
  spynode.cpp
  vertexnode.cpp
)

file(GLOB HEADERS
  constantnode.h
  composenode.h
  decomposenode.h
  decomposecolornode.h
  composecolornode.h
  fragmentnode.h
  mathnode.h
  referencenode.h
  spynode.h
  vertexnode.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

