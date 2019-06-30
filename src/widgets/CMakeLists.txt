
file(GLOB SOURCES
  codeedit.cpp
  coordinateedit.cpp
  filepathedit.cpp
  keysequenceedit.cpp
  pointdialog.cpp
  pointedit.cpp
  referencelineedit.cpp
)

file(GLOB HEADERS
  codeedit.h
  coordinateedit.h
  filepathedit.h
  keysequenceedit.h
  numericedit.h
  pointdialog.h
  pointedit.h
  referencelineedit.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

