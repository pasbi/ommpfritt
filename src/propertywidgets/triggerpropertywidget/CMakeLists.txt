
file(GLOB SOURCES
  triggerpropertyconfigwidget.cpp
  triggerpropertywidget.cpp
)

file(GLOB HEADERS
  triggerpropertyconfigwidget.h
  triggerpropertywidget.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

