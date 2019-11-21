
file(GLOB SOURCES
  curvemanager.cpp
  curvemanagerwidget.cpp
  curvemanagertitlebar.cpp
  curvemanagerquickaccessdelegate.cpp
  curvetree.cpp
)

file(GLOB HEADERS
  curvemanager.h
  curvemanagerwidget.h
  curvemanagertitlebar.h
  curvemanagerquickaccessdelegate.h
  curvetree.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

