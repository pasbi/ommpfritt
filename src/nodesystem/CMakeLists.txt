target_sources(libommpfritt PRIVATE
  node.cpp
  node.h
  propertyport.cpp
  propertyport.h
  nodemodel.cpp
  nodemodel.h
  nodecompiler.cpp
  nodecompiler.h
  nodecompilerpython.cpp
  nodecompilerpython.h
  nodecompilerglsl.cpp
  nodecompilerglsl.cpp
  nodesowner.cpp
  nodesowner.h
  port.cpp
  port.h
  ordinaryport.cpp
  ordinaryport.h
)

add_subdirectory(nodes)
