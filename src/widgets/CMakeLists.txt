
file(GLOB SOURCES
  animationbutton.cpp
  codeedit.cpp
  combobox.cpp
  coordinateedit.cpp
  filepathedit.cpp
  keysequenceedit.cpp
  multitabbar.cpp
  pointdialog.cpp
  pointedit.cpp
  referencelineedit.cpp
  styleditemview.cpp
)

file(GLOB HEADERS
  animationbutton.h
  codeedit.h
  combobox.h
  coordinateedit.h
  filepathedit.h
  keysequenceedit.h
  multitabbar.h
  numericedit.h
  pointdialog.h
  pointedit.h
  referencelineedit.h
  styleditemview.h
)

add_subdirectory(colorwidget)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

