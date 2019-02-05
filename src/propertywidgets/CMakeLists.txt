file (GLOB SOURCES
  "abstractedit.cpp"
  "propertyconfigwidget.cpp"
  "propertyconfigwidgetsregister.cpp"
  "propertywidget.cpp"
  "propertywidgetregister.cpp"
)

target_sources(libommpfritt PRIVATE ${SOURCES})

add_subdirectory("boolpropertywidget")
add_subdirectory("colorpropertywidget")
add_subdirectory("numericpropertywidget")
add_subdirectory("optionspropertywidget")
add_subdirectory("referencepropertywidget")
add_subdirectory("stringpropertywidget")
add_subdirectory("triggerpropertywidget")
add_subdirectory("vectorpropertywidget")

