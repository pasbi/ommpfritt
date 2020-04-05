
file(GLOB SOURCES
  boolproperty.cpp
  colorproperty.cpp
  floatproperty.cpp
  integerproperty.cpp
  numericproperty.cpp
  optionsproperty.cpp
  property.cpp
  referenceproperty.cpp
  stringproperty.cpp
  splineproperty.cpp
  triggerproperty.cpp
  vectorproperty.cpp
)

file(GLOB HEADERS
  boolproperty.h
  colorproperty.h
  floatproperty.h
  integerproperty.h
  numericproperty.h
  optionsproperty.h
  property.h
  referenceproperty.h
  stringproperty.h
  splineproperty.h
  triggerproperty.h
  typedproperty.h
  vectorproperty.h
)

add_subdirectory(propertygroups)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

