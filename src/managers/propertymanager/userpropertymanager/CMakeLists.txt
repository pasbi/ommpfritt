file (GLOB SOURCES
  "boolpropertyconfigwidget.cpp"
  "colorpropertyconfigwidget.cpp"
  "floatpropertyconfigwidget.cpp"
  "integerpropertyconfigwidget.cpp"
  "propertyitem.cpp"
  "propertyconfigwidget.cpp"
  "propertyconfigwidgetsregister.cpp"
  "referencepropertyconfigwidget.cpp"
  "stringpropertyconfigwidget.cpp"
  "transformationpropertyconfigwidget.cpp"
  "userpropertydialog.cpp"
)

target_sources(libommpfritt PRIVATE ${SOURCES})
