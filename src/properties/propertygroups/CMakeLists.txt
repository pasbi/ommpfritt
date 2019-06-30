
file(GLOB SOURCES
  fontproperties.cpp
  markerproperties.cpp
  propertygroup.cpp
  textoptionproperties.cpp
)

file(GLOB HEADERS
  fontproperties.h
  markerproperties.h
  propertygroup.h
  textoptionproperties.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

