
file(GLOB SOURCES
  command.cpp
  copycommand.cpp
  modifypointscommand.cpp
  movecommand.cpp
  movetagscommand.cpp
  objectselectioncommand.cpp
  objectstransformationcommand.cpp
  pointstransformationcommand.cpp
  propertycommand.cpp
  removecommand.cpp
  userpropertycommand.cpp
)

file(GLOB HEADERS
  addcommand.h
  command.h
  copycommand.h
  modifypointscommand.h
  movecommand.h
  movetagscommand.h
  objectselectioncommand.h
  objectstransformationcommand.h
  pointstransformationcommand.h
  propertycommand.h
  removecommand.h
  userpropertycommand.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

