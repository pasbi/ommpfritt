file (GLOB SOURCES
  "addobjectcommand.cpp"
  "attachtagcommand.cpp"
  "command.cpp"
  "copycommand.cpp"
  "movecommand.cpp"
  "newstylecommand.cpp"
  "objectstransformationcommand.cpp"
  "propertycommand.cpp"
  "removeobjectscommand.cpp"
  "removestylescommand.cpp"
)
target_sources(libommpfritt PRIVATE ${SOURCES})