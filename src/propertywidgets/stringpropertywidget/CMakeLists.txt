
file(GLOB SOURCES
  fontcombobox.cpp
  stringpropertyconfigwidget.cpp
  stringpropertywidget.cpp
  texteditadapter.cpp
)

file(GLOB HEADERS
  fontcombobox.h
  stringpropertyconfigwidget.h
  stringpropertywidget.h
  texteditadapter.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

