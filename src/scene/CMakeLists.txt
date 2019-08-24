
file(GLOB SOURCES
  itemmodeladapter.cpp
  list.cpp
  objecttree.cpp
  pointselection.cpp
  propertyownermimedata.cpp
  scene.cpp
  structure.cpp
  stylelist.cpp
)

file(GLOB HEADERS
  cachedgetter.h
  contextes_fwd.h
  contextes.h
  itemmodeladapter.h
  list.h
  objecttree.h
  pointselection.h
  propertyownermimedata.h
  scene.h
  structure.h
  stylelist.h
)

add_subdirectory(history)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

