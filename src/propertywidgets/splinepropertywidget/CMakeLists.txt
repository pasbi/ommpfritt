
file(GLOB SOURCES
  splinepropertywidget.cpp
  splinepropertyconfigwidget.cpp
)

file(GLOB HEADERS
  splinepropertywidget.h
  splinepropertyconfigwidget.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

