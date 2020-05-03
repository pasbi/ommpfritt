target_sources(libommpfritt PRIVATE
  cycleguard.cpp
  cycleguard.h
  itemmodeladapter.cpp
  contextes_fwd.h
  contextes.h
  itemmodeladapter.h
  list.cpp
  list.h
  messagebox.cpp
  messagebox.h
  objecttree.h
  objecttree.cpp
  pointselection.h
  pointselection.cpp
  propertyownermimedata.h
  propertyownermimedata.cpp
  scene.h
  scene.cpp
  structure.h
  structure.cpp
  stylelist.h
  stylelist.cpp
  taglist.h
  taglist.cpp
)

add_subdirectory(history)
