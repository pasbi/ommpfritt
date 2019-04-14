file (GLOB SOURCES
  "actions.cpp"
  "application.cpp"
  "exportdialog.cpp"
  "mainmenubar.cpp"
  "mainwindow.cpp"
  "toolbar.cpp"
)

target_sources(libommpfritt PRIVATE ${SOURCES})
add_subdirectory(viewport)
