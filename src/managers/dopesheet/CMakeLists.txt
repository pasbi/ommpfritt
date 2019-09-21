
file(GLOB SOURCES
  dopesheetmanager.cpp
  dopesheetview.cpp
)

file(GLOB HEADERS
  dopesheetmanager.h
  dopesheetview.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

