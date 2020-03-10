
file(GLOB SOURCES
  node.cpp
  nodemodel.cpp
  nodecompiler.cpp
  nodecompilerpython.cpp
  nodecompilerglsl.cpp
  nodesowner.cpp
  port.cpp
  propertyport.cpp
  ordinaryport.cpp
)

file(GLOB HEADERS
  node.h
  nodemodel.h
  nodecompiler.h
  nodecompilerpython.h
  nodecompilerglsl.cpp
  nodesowner.h
  port.h
  propertyport.cpp
  ordinaryport.h
)

add_subdirectory(nodes)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

