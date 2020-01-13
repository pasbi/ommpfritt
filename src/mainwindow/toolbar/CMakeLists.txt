
file(GLOB SOURCES
  toolbar.cpp
  toolbardialog.cpp
  toolbaractiontreewidget.cpp
  toolbaritemmodel.cpp
  toolbaritemview.cpp
)

file(GLOB HEADERS
  toolbar.h
  toolbardialog.h
  toolbaractiontreewidget.h
  toolbaritemmodel.h
  toolbaritemview.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

