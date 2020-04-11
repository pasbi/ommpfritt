
file(GLOB SOURCES
  optionsedit.cpp
  optionpropertyconfigwidget.cpp
  optionpropertywidget.cpp
)

file(GLOB HEADERS
  optionsedit.h
  optionpropertyconfigwidget.h
  optionpropertywidget.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

