
file(GLOB SOURCES
  actions.cpp
  application.cpp
  exportdialog.cpp
  iconprovider.cpp
  mainwindow.cpp
  toolbar.cpp
)

file(GLOB HEADERS
  actions.h
  application.h
  exportdialog.h
  iconprovider.h
  mainwindow.h
  toolbar.h
)

add_subdirectory(viewport)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

