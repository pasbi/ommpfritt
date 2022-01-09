target_sources(libommpfritt PRIVATE
  common.cpp
  common.h
  node.cpp
  node.h
  nodecompiler.cpp
  nodecompiler.h
  nodecompilerglsl.cpp
  nodecompilerglsl.h
  nodecompilerpython.cpp
  nodecompilerpython.h
  nodemodel.cpp
  nodemodel.h
  nodesowner.cpp
  nodesowner.h
  ordinaryport.cpp
  ordinaryport.h
  port.cpp
  port.h
  propertyport.cpp
  propertyport.h
  statement.cpp
  statement.h
)

add_subdirectory(nodes)
