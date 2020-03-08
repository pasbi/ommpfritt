
file(GLOB SOURCES
  imagecache.cpp
  painter.cpp
  offscreenrenderer.cpp
  style.cpp
  styleiconengine.cpp
  texture.cpp
)

file(GLOB HEADERS
  imagecache.h
  painter.h
  offscreenrenderer.h
  style.h
  styleiconengine.h
  texture.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

