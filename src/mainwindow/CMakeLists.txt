
file(GLOB SOURCES
  actions.cpp
  application.cpp
  exportdialog.cpp
  iconprovider.cpp
  mainwindow.cpp
  options.cpp
)

file(GLOB HEADERS
  actions.h
  application.h
  exportdialog.h
  iconprovider.h
  languageskinmenu.h
  mainwindow.h
  options.h
)

add_subdirectory(viewport)
add_subdirectory(toolbar)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

