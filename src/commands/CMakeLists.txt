
file(GLOB SOURCES
  command.cpp
  copycommand.cpp
  keyframecommand.cpp
  modifypointscommand.cpp
  movecommand.cpp
  movetagscommand.cpp
  namedcolorscommand.cpp
  objectselectioncommand.cpp
  objectstransformationcommand.cpp
  pointstransformationcommand.cpp
  propertycommand.cpp
  removecommand.cpp
  trackcommand.cpp
  userpropertycommand.cpp
)

file(GLOB HEADERS
  addcommand.h
  command.h
  copycommand.h
  keyframecommand.h
  modifypointscommand.h
  movecommand.h
  movetagscommand.h
  namedcolorscommand.h
  objectselectioncommand.h
  objectstransformationcommand.h
  pointstransformationcommand.h
  propertycommand.h
  removecommand.h
  trackcommand.h
  userpropertycommand.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

