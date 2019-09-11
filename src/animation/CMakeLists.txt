
file(GLOB SOURCES
  abstractfcurve.cpp
  animator.cpp
  fcurve.cpp
  keyvalue.cpp
)

file(GLOB HEADERS
  abstractfcurve.h
  animator.h
  fcurve.h
  keyvalue.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

