
file(GLOB SOURCES
  abstractserializer.cpp
  jsonserializer.cpp
  serializerregister.cpp
)

file(GLOB HEADERS
  abstractserializer.h
  jsonserializer.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

