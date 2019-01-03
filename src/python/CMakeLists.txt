file (GLOB SOURCES
  "common.cpp"
  "objecttransformationwrapper.cpp"
  "objectwrapper.cpp"
  "pointwrapper.cpp"
  "propertyownerwrapper.cpp"
  "pythonengine.cpp"
  "pywrapper.cpp"
  "scenewrapper.cpp"
  "stylewrapper.cpp"
  "tagwrapper.cpp"
  "scripttagwrapper.cpp"
  "wrapperregister.cpp"
)
target_sources(libommpfritt PRIVATE ${SOURCES})

