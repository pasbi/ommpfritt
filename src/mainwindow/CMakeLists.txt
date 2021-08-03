target_sources(libommpfritt PRIVATE
  pathactions.cpp
  pathactions.h
  exportdialog.cpp
  exportdialog.h
  exportoptions.cpp
  exportoptions.h
  exporter.cpp
  exporter.h
  iconprovider.cpp
  iconprovider.h
  mainwindow.cpp
  mainwindow.h
  gpl3.h
)

add_subdirectory(viewport)
add_subdirectory(toolbar)
