
file(GLOB SOURCES
  node.cpp
  nodemanager.cpp
  nodemanagertitlebar.cpp
  nodemodel.cpp
  nodeview.cpp
  nodecompiler.cpp
  nodetopythoncompiler.cpp
  nodesowner.cpp
  port.cpp
  propertyport.cpp
  ordinaryport.cpp
)

file(GLOB HEADERS
  node.h
  nodemanager.h
  nodemanagertitlebar.h
  nodemodel.h
  nodeview.h
  nodecompiler.h
  nodetopythoncompiler.h
  nodesowner.h
  port.h
  propertyport.cpp
  ordinaryport.h
)

add_subdirectory(nodes)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

