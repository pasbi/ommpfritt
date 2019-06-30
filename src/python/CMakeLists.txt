
file(GLOB SOURCES
  objectwrapper.cpp
  pathwrapper.cpp
  pointwrapper.cpp
  propertyownerwrapper.cpp
  pythonengine.cpp
  pythonstreamredirect.cpp
  pywrapper.cpp
  scenewrapper.cpp
  scripttagwrapper.cpp
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
  pythonstreamredirect.h
  pywrapper.h
  scenewrapper.h
  scripttagwrapper.h
  stylewrapper.h
  tagwrapper.h
  wrapperfactory.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

