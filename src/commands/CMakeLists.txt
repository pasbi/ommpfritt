file (GLOB SOURCES
  "addobjectcommand.cpp"
  "attachtagcommand.cpp"
  "command.cpp"
  "copyobjectscommand.cpp"
  "moveobjectscommand.cpp"
  "newstylecommand.cpp"
  "objectstransformationcommand.cpp"
  "propertycommand.cpp"
  "removeobjectscommand.cpp"
)
target_sources(libommpfritt PRIVATE ${SOURCES})