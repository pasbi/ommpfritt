file (GLOB SOURCES
  "command.cpp"
  "copycommand.cpp"
  "movecommand.cpp"
  "modifytangentscommand.cpp"
  "newstylecommand.cpp"
  "objectstransformationcommand.cpp"
  "pointstransformationcommand.cpp"
  "propertycommand.cpp"
  "removecommand.cpp"
  "userpropertyconfigcommand.cpp"
)
target_sources(libommpfritt PRIVATE ${SOURCES})
