
file(GLOB SOURCES
  optionsedit.cpp
  optionspropertyconfigwidget.cpp
  optionspropertywidget.cpp
)

file(GLOB HEADERS
  optionsedit.h
  optionspropertyconfigwidget.h
  optionspropertywidget.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

