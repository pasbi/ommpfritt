file (GLOB SOURCES
  "abstractproceduralpath.cpp"
  "cloner.cpp"
  "ellipse.cpp"
  "empty.cpp"
  "imageobject.cpp"
  "instance.cpp"
  "line.cpp"
  "mirror.cpp"
  "object.cpp"
  "objectregister.cpp"
  "outline.cpp"
  "path.cpp"
  "proceduralpath.cpp"
  "text.cpp"
  "view.cpp"
)
target_sources(libommpfritt PRIVATE ${SOURCES})
