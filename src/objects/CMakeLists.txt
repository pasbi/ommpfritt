
file(GLOB SOURCES
  abstractproceduralpath.cpp
  cloner.cpp
  ellipse.cpp
  empty.cpp
  imageobject.cpp
  instance.cpp
  line.cpp
  mirror.cpp
  object.cpp
  objectregister.cpp
  outline.cpp
  path.cpp
  proceduralpath.cpp
  rectangleobject.cpp
  text.cpp
  tip.cpp
  view.cpp
)

file(GLOB HEADERS
  abstractproceduralpath.h
  cloner.h
  ellipse.h
  empty.h
  imageobject.h
  instance.h
  line.h
  mirror.h
  object.h
  outline.h
  path.h
  proceduralpath.h
  rectangleobject.h
  text.h
  tip.h
  view.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

