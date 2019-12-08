
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

add_subdirectory(nodes)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

