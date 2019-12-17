
file(GLOB SOURCES
  node.cpp
  nodemanager.cpp
  nodemodel.cpp
  nodeview.cpp
  nodecompiler.cpp
  nodetopythoncompiler.cpp
  port.cpp
  propertyport.cpp
)

file(GLOB HEADERS
  node.h
  nodemanager.h
  nodemodel.h
  nodeview.h
  nodecompiler.h
  nodetopythoncompiler.h
  port.h
  propertyport.cpp
)

add_subdirectory(nodes)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

