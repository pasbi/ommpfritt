
file(GLOB SOURCES
  abstractcolorcomponentwidget.cpp
  colorcircle.cpp
  colordialog.cpp
  colorcomponentspinbox.cpp
  colorpicker.cpp
  colorslab.cpp
  colorslider.cpp
  colorwidget.cpp
  namedcolorsdialog.cpp
)

file(GLOB HEADERS
  abstractcolorcomponentwidget.h
  colorcircle.h
  colordialog.h
  colorcomponentspinbox.h
  colorpicker.h
  colorslab.h
  colorslider.h
  colorwidget.h
  namedcolorsdialog.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

