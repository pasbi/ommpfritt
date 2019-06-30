
file(GLOB SOURCES
  vectorpropertyconfigwidget.cpp
)

file(GLOB HEADERS
  vectorpropertyconfigwidget.h
  vectorpropertywidget.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

