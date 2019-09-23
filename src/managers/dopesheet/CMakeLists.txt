
file(GLOB SOURCES
  dopesheetmanager.cpp
  dopesheetview.cpp
  trackviewdelegate.cpp
)

file(GLOB HEADERS
  dopesheetmanager.h
  dopesheetview.h
  trackviewdelegate.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

