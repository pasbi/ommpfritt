
file(GLOB SOURCES
  node.cpp
  nodemanager.cpp
  nodemodel.cpp
  nodeview.cpp
  panzoomcontroller.cpp
  port.cpp
)

file(GLOB HEADERS
  node.h
  nodemanager.h
  nodemodel.h
  nodeview.h
  panzoomcontroller.h
  port.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

