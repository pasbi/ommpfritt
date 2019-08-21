
file(GLOB SOURCES
  itemmodeladapter.cpp
  list.cpp
  objecttreeadapter.cpp
  pointselection.cpp
  propertyownermimedata.cpp
  scene.cpp
  structure.cpp
  stylelist.cpp
  tree.cpp
)

file(GLOB HEADERS
  cachedgetter.h
  contextes_fwd.h
  contextes.h
  itemmodeladapter.h
  list.h
  objecttreeadapter.h
  pointselection.h
  propertyownermimedata.h
  scene.h
  structure.h
  stylelist.h
  tree.h
)

add_subdirectory(history)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

