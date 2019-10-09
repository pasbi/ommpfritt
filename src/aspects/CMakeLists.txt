
file(GLOB SOURCES
  abstractpropertyowner.cpp
  propertyowner.cpp
  settingstree.cpp
  settingstreeitem.cpp
  treeelement.cpp
)

file(GLOB HEADERS
  abstractpropertyowner.h
  propertyowner.h
  serializable.h
  settingstree.h
  settingstreeitem.h
  treeelement.h
  typed.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

