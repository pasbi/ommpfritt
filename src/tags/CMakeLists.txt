
file(GLOB SOURCES
  pathtag.cpp
  nodestag.cpp
  scripttag.cpp
  styletag.cpp
  tag.cpp
)

file(GLOB HEADERS
  pathtag.h
  nodestag.h
  scripttag.h
  styletag.h
  tag.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

