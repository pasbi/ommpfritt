
file(GLOB SOURCES
  numericpropertywidget.cpp
)

file(GLOB HEADERS
  numericmultivalueedit.h
  numericpropertyconfigwidget.h
  numericpropertywidget.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

