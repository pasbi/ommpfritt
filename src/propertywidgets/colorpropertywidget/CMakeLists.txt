
file(GLOB SOURCES
  coloredit.cpp
  colorpropertywidget.cpp
)

file(GLOB HEADERS
  coloredit.h
  colorpropertyconfigwidget.h
  colorpropertywidget.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

