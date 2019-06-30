
file(GLOB SOURCES
  pathtag.cpp
  scripttag.cpp
  styletag.cpp
  tag.cpp
  tagregister.cpp
)

file(GLOB HEADERS
  pathtag.h
  scripttag.h
  styletag.h
  tag.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

