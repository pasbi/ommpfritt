target_sources(libommpfritt PRIVATE
  pathactions.cpp
  pathactions.h
  application.cpp
  application.h
  exportdialog.cpp
  exportdialog.h
  iconprovider.cpp
  iconprovider.h
  mainwindow.cpp
  mainwindow.h
  options.cpp
  options.h
)

add_subdirectory(viewport)
add_subdirectory(toolbar)
