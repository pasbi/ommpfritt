file (GLOB SOURCES
  "application.cpp"
  "mainmenubar.cpp"
  "mainwindow.cpp"
  "toolbar.cpp"
)
target_sources(libommpfritt PRIVATE ${SOURCES})

add_subdirectory(viewport)
