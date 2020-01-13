
file(GLOB SOURCES
  actions.cpp
  application.cpp
  exportdialog.cpp
  iconprovider.cpp
  mainwindow.cpp
  options.cpp
  toolbar.cpp
  toolbardialog.cpp
  toolbaractiontreewidget.cpp
  toolbaritemmodel.cpp
  toolbaritemview.cpp
)

file(GLOB HEADERS
  actions.h
  application.h
  exportdialog.h
  iconprovider.h
  languageskinmenu.h
  mainwindow.h
  options.h
  toolbar.h
  toolbardialog.h
  toolbaractiontreewidget.h
  toolbaritemmodel.h
  toolbaritemview.h
)

add_subdirectory(viewport)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

