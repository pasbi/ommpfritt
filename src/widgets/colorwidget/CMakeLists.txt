
file(GLOB SOURCES
  abstractcolorcomponentwidget.cpp
  colorcircle.cpp
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
  colorcomponentspinbox.h
  colorpicker.h
  colorslab.h
  colorslider.h
  colorwidget.h
  namedcolorsdialog.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

