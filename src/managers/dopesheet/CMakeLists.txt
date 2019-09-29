
file(GLOB SOURCES
  dopesheetmanager.cpp
  dopesheetview.cpp
  dopesheetheader.cpp
  trackviewdelegate.cpp
)

file(GLOB HEADERS
  dopesheetmanager.h
  dopesheetview.h
  dopesheetheader.h
  trackviewdelegate.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

