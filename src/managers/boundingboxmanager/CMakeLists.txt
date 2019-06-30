
file(GLOB SOURCES
  anchorwidget.cpp
  boundingboxmanager.cpp
)

file(GLOB HEADERS
  anchorwidget.h
  boundingboxmanager.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

