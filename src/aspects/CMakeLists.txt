
file(GLOB SOURCES
  abstractpropertyowner.cpp
  autoconnectiondeleter.cpp
  propertyowner.cpp
  treeelement.cpp
)

file(GLOB HEADERS
  abstractpropertyowner.h
  autoconnectiondeleter.h
  propertyowner.h
  serializable.h
  treeelement.h
  typed.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

