
file(GLOB SOURCES
  objectwrapper.cpp
  pathwrapper.cpp
  pointwrapper.cpp
  propertyownerwrapper.cpp
  pythonengine.cpp
  pywrapper.cpp
  scenewrapper.cpp
  scripttagwrapper.cpp
  scopedinterpreterwrapper.cpp
  stylewrapper.cpp
  tagwrapper.cpp
  wrapperregister.cpp
)

file(GLOB HEADERS
  objectwrapper.h
  pathwrapper.h
  pointwrapper.h
  propertyownerwrapper.h
  pythonengine.h
  pywrapper.h
  scenewrapper.h
  scopedinterpreterwrapper.h
  scripttagwrapper.h
  stylewrapper.h
  tagwrapper.h
  wrapperfactory.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

