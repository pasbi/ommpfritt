
file(GLOB SOURCES
  historymodel.cpp
)

file(GLOB HEADERS
  historymodel.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

