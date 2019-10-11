
file(GLOB SOURCES
  abstractpropertyowner.cpp
  propertyowner.cpp
  treeelement.cpp
)

file(GLOB HEADERS
  abstractpropertyowner.h
  propertyowner.h
  serializable.h
  treeelement.h
  typed.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

