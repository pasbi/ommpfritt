target_sources(libommpfritt PRIVATE
  common.cpp
  common.h
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
  statement.cpp
  statement.h
)

add_subdirectory(nodes)
