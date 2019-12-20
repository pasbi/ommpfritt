
file(GLOB SOURCES
  command.cpp
  copycommand.cpp
  forwardingportcommand.cpp
  keyframecommand.cpp
  modifypointscommand.cpp
  movecommand.cpp
  movetagscommand.cpp
  namedcolorscommand.cpp
  nodecommand.cpp
  objectselectioncommand.cpp
  objectstransformationcommand.cpp
  pointstransformationcommand.cpp
  propertycommand.cpp
  removecommand.cpp
  setinterpolationcommand.cpp
  trackcommand.cpp
  userpropertycommand.cpp
)

file(GLOB HEADERS
  addcommand.h
  command.h
  copycommand.h
  forwardingportcommand.h
  keyframecommand.h
  modifypointscommand.h
  movecommand.h
  movetagscommand.h
  namedcolorscommand.h
  nodecommand.h
  objectselectioncommand.h
  objectstransformationcommand.h
  pointstransformationcommand.h
  propertycommand.h
  removecommand.h
  setinterpolationcommand.h
  trackcommand.h
  userpropertycommand.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

