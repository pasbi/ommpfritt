
file(GLOB SOURCES
  referencepropertyconfigwidget.cpp
  referencepropertywidget.cpp
)

file(GLOB HEADERS
  referencepropertyconfigwidget.h
  referencepropertywidget.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

