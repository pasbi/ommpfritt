
file(GLOB SOURCES
  node.cpp
  nodemanager.cpp
  nodemanagertitlebar.cpp
  nodemodel.cpp
  nodeview.cpp
  nodecompiler.cpp
  nodecompilerpython.cpp
  nodecompilerglsl.cpp
  nodemimedata.cpp
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
  nodecompilerpython.h
  nodecompilerglsl.cpp
  nodemimedata.h
  nodesowner.h
  port.h
  propertyport.cpp
  ordinaryport.h
)

add_subdirectory(nodes)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

