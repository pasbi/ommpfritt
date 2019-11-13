
file(GLOB SOURCES
  curvemanager.cpp
  curvemanagerwidget.cpp
  curvemanagertitlebar.cpp
  curvetimelinecanvas.cpp
)

file(GLOB HEADERS
  curvemanager.h
  curvemanagerwidget.h
  curvemanagertitlebar.h
  curvetimelinecanvas.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

