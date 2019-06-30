
file(GLOB SOURCES
  stylelistview.cpp
  stylemanager.cpp
)

file(GLOB HEADERS
  stylelistview.h
  stylemanager.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

