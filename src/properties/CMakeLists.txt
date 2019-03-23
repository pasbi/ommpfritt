file (GLOB SOURCES
  "boolproperty.cpp"
  "colorproperty.cpp"
  "floatproperty.cpp"
  "integerproperty.cpp"
  "optionsproperty.cpp"
  "property.cpp"
  "propertyregister.cpp"
  "referenceproperty.cpp"
  "stringproperty.cpp"
  "transformationproperty.cpp"
  "triggerproperty.cpp"
  "typedproperty.cpp"
  "vectorproperty.cpp"
)

target_sources(libommpfritt PRIVATE ${SOURCES})

add_subdirectory(propertygroups)
