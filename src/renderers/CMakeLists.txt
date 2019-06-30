
file(GLOB SOURCES
  imagecache.cpp
  painter.cpp
  style.cpp
  styleiconengine.cpp
)

file(GLOB HEADERS
  imagecache.h
  painter.h
  style.h
  styleiconengine.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

