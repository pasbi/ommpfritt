file (GLOB SOURCES
  "codeedit.cpp"
  "coordinateedit.cpp"
  "filepathedit.cpp"
  "keysequenceedit.cpp"
  "numericedit.h"  # required for moc
  "pointdialog.cpp"
  "pointedit.cpp"
  "referencelineedit.cpp"
)

target_sources(libommpfritt PRIVATE ${SOURCES})
