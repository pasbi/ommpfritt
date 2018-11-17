file (GLOB SOURCES
  "addobjectcommand.cpp"
  "command.cpp"
  "copyobjectscommand.cpp"
  "moveobjectscommand.cpp"
  "propertycommand.cpp"
  "removeobjectscommand.cpp"
)
target_sources(libommpfritt PRIVATE ${SOURCES})