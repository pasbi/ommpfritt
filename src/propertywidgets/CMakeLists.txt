
file(GLOB SOURCES
  propertyconfigwidget.cpp
  propertyconfigwidgetsregister.cpp
  propertywidget.cpp
  propertywidgetregister.cpp
)

file(GLOB HEADERS
  abstractedit.h
  multivalueedit.h
  propertyconfigwidget.h
  propertywidget.h
)

add_subdirectory(boolpropertywidget)
add_subdirectory(colorpropertywidget)
add_subdirectory(numericpropertywidget)
add_subdirectory(optionspropertywidget)
add_subdirectory(referencepropertywidget)
add_subdirectory(stringpropertywidget)
add_subdirectory(splinepropertywidget)
add_subdirectory(triggerpropertywidget)
add_subdirectory(vectorpropertywidget)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

