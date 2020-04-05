
file(GLOB SOURCES
  splinepropertywidget.cpp
  splinepropertyconfigwidget.cpp
  splinewidget.cpp
)

file(GLOB HEADERS
  splinepropertywidget.h
  splinepropertyconfigwidget.h
  splinewidget.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

