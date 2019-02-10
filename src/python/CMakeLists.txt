file (GLOB SOURCES
  "objectwrapper.cpp"
  "pathwrapper.cpp"
  "pointwrapper.cpp"
  "propertyownerwrapper.cpp"
  "pythonengine.cpp"
  "pythonstreamredirect.cpp"
  "pywrapper.cpp"
  "scenewrapper.cpp"
  "stylewrapper.cpp"
  "tagwrapper.cpp"
  "scripttagwrapper.cpp"
  "wrapperregister.cpp"
)
target_sources(libommpfritt PRIVATE ${SOURCES})

