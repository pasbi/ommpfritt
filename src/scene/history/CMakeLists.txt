
file(GLOB SOURCES
  historymodel.cpp
  macro.cpp
)

file(GLOB HEADERS
  historymodel.h
  macro.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

