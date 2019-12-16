
file(GLOB SOURCES
  constantnode.cpp
  inputnode.cpp
  outputnode.cpp
  gradientnode.cpp
  nodesregister.cpp
  spynode.cpp
)

file(GLOB HEADERS
  constantnode.h
  inputnode.h
  outputnode.h
  gradientnode.h
  spynode.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

