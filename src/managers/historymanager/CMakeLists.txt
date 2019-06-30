
file(GLOB SOURCES
  historymanager.cpp
)

file(GLOB HEADERS
  historymanager.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

