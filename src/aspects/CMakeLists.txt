
file(GLOB SOURCES
  autoconnectiondeleter.cpp
  propertyowner.cpp
  serializable.cpp
  treeelement.cpp
)

file(GLOB HEADERS
  autoconnectiondeleter.h
  propertyowner.h
  serializable.h
  treeelement.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

