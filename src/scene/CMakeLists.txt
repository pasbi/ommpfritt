
file(GLOB SOURCES
  animator.cpp
  cycleguard.cpp
  itemmodeladapter.cpp
  list.cpp
  messagebox.cpp
  objecttree.cpp
  pointselection.cpp
  propertyownermimedata.cpp
  scene.cpp
  structure.cpp
  stylelist.cpp
  taglist.cpp
)

file(GLOB HEADERS
  animator.h
  cycleguard.h
  cachedgetter.h
  contextes_fwd.h
  contextes.h
  itemmodeladapter.h
  list.h
  messagebox.h
  objecttree.h
  pointselection.h
  propertyownermimedata.h
  scene.h
  structure.h
  stylelist.h
  taglist.h
)

add_subdirectory(history)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

