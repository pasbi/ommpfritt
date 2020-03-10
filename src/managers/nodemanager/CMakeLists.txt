
file(GLOB SOURCES
  nodeitem.cpp
  nodemanager.cpp
  nodemanagertitlebar.cpp
  nodescene.cpp
  nodeview.cpp
  nodemimedata.cpp
  nodesowner.cpp
  portitem.cpp
)

file(GLOB HEADERS
  nodeitem.h
  nodemanager.h
  nodemanagertitlebar.h
  nodescene.h
  nodeview.h
  nodesowner.h
  portitem.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})
