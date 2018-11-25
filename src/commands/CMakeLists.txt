file (GLOB SOURCES
  "addobjectcommand.cpp"
  "attachtagcommand.cpp"
  "command.cpp"
  "copyobjectscommand.cpp"
  "moveobjectscommand.cpp"
  "propertycommand.cpp"
  "removeobjectscommand.cpp"
  "objectstransformationcommand.cpp"
)
target_sources(libommpfritt PRIVATE ${SOURCES})