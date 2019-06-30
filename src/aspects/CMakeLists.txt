
file(GLOB SOURCES
  propertyowner.cpp
  serializable.cpp
  treeelement.cpp
)

file(GLOB HEADERS
  propertyowner.h
  serializable.h
  treeelement.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

