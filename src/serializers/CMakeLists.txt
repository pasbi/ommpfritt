target_sources(libommpfritt PRIVATE
  array.cpp
  array.h
  abstractserializer.cpp
  abstractserializer.h
  abstractdeserializer.cpp
  abstractdeserializer.h
  deserializerworker.cpp
  deserializerworker.h
  serializerworker.cpp
  serializerworker.h
)

add_subdirectory(json)
