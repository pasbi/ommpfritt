
file(GLOB SOURCES
  stringpropertyconfigwidget.cpp
  stringpropertywidget.cpp
  texteditadapter.cpp
)

file(GLOB HEADERS
  stringpropertyconfigwidget.h
  stringpropertywidget.h
  texteditadapter.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

