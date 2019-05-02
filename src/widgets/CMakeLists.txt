file (GLOB SOURCES
  "codeedit.cpp"
  "coordinateedit.cpp"
  "filepathedit.cpp"
  "referencelineedit.cpp"
  "pointdialog.cpp"
  "pointedit.cpp"
  "numericedit.h"  # required for moc
)

target_sources(libommpfritt PRIVATE ${SOURCES})
