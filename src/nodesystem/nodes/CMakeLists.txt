
file(GLOB SOURCES
  constantnode.cpp
  composenode.cpp
  colorconvertnode.cpp
  decomposenode.cpp
  decomposecolornode.cpp
  composecolornode.cpp
  fragmentnode.cpp
  functionnode.cpp
  function2node.cpp
  interpolatenode.cpp
  linepatternnode.cpp
  mathnode.cpp
  referencenode.cpp
  spynode.cpp
  vertexnode.cpp
)

file(GLOB HEADERS
  constantnode.h
  composenode.h
  colorconvertnode.h
  decomposenode.h
  decomposecolornode.h
  composecolornode.h
  fragmentnode.h
  functionnode.h
  function2node.h
  interpolatenode.h
  linepatternnode.h
  mathnode.h
  referencenode.h
  spynode.h
  vertexnode.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

