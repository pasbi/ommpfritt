
file(GLOB SOURCES
)

file(GLOB HEADERS
  vectorpropertyconfigwidget.h
  vectorpropertywidget.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

