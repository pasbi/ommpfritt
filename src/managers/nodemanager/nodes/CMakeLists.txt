
file(GLOB SOURCES
  inputnode.cpp
  outputnode.cpp
  gradientnode.cpp
  nodesregister.cpp
  spynode.cpp
)

file(GLOB HEADERS
  inputnode.h
  outputnode.h
  gradientnode.h
  spynode.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

