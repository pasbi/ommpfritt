file (GLOB SOURCES
  "application.cpp"
  "mainmenubar.cpp"
  "mainwindow.cpp"
)
target_sources(libommpfritt PRIVATE ${SOURCES})

add_subdirectory(viewport)
