
file(GLOB SOURCES
  animationbutton.cpp
  codeedit.cpp
  combobox.cpp
  coordinateedit.cpp
  filepathedit.cpp
  itemproxyview.cpp
  keysequenceedit.cpp
  multitabbar.cpp
  numericedit.cpp
  pointdialog.cpp
  pointedit.cpp
  referencelineedit.cpp
  splinewidget.cpp
)

file(GLOB HEADERS
  animationbutton.h
  codeedit.h
  combobox.h
  coordinateedit.h
  filepathedit.h
  temproxyview.h
  keysequenceedit.h
  multitabbar.h
  numericedit.h
  pointdialog.h
  pointedit.h
  referencelineedit.h
  splinewidget.h
)

add_subdirectory(colorwidget)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

