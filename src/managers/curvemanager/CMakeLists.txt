
file(GLOB SOURCES
  curvemanager.cpp
  curvemanagerwidget.cpp
  curvemanagertitlebar.cpp
)

file(GLOB HEADERS
  curvemanager.h
  curvemanagerwidget.h
  curvemanagertitlebar.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

