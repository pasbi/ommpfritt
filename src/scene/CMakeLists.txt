
file(GLOB SOURCES
  itemmodeladapter.cpp
  listadapter.cpp
  list.cpp
  objecttreeadapter.cpp
  pointselection.cpp
  propertyownermimedata.cpp
  scene.cpp
  structure.cpp
  tree.cpp
)

file(GLOB HEADERS
  abstractstructureobserver.h
  cachedgetter.h
  contextes_fwd.h
  contextes.h
  itemmodeladapter.h
  listadapter.h
  list.h
  objecttreeadapter.h
  pointselection.h
  propertyownermimedata.h
  scene.h
  structure.h
  tree.h
)

add_subdirectory(history)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

