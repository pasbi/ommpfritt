
file(GLOB SOURCES
  abstractpropertyowner.cpp
  animatablepropertyowner.cpp
  autoconnectiondeleter.cpp
  propertyowner.cpp
  treeelement.cpp
)

file(GLOB HEADERS
  abstractpropertyowner.h
  animatablepropertyowner.h
  autoconnectiondeleter.h
  propertyowner.h
  serializable.h
  treeelement.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

