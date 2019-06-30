
file(GLOB SOURCES
  boolpropertyconfigwidget.cpp
  boolpropertywidget.cpp
  checkbox.cpp
)

file(GLOB HEADERS
  boolpropertyconfigwidget.h
  boolpropertywidget.h
  checkbox.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

