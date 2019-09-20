
file(GLOB SOURCES
  dopesheet.cpp
  dopesheetmanager.cpp
  dopesheetview.cpp
)

file(GLOB HEADERS
  dopesheet.h
  dopesheetmanager.h
  dopesheetview.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

