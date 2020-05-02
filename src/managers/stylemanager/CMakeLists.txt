
file(GLOB SOURCES
  stylelistviewitemdelegate.cpp
  stylelistview.cpp
  stylemanager.cpp
)

file(GLOB HEADERS
  stylelistviewitemdelegate.h
  stylelistview.h
  stylemanager.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

