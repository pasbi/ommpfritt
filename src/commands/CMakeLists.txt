file (GLOB SOURCES
  "command.cpp"
  "copycommand.cpp"
  "movecommand.cpp"
  "newstylecommand.cpp"
  "objectstransformationcommand.cpp"
  "propertycommand.cpp"
  "removecommand.cpp"
  "userpropertyconfigcommand.cpp"
)
target_sources(libommpfritt PRIVATE ${SOURCES})
