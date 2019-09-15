
file(GLOB SOURCES
  animator.cpp
  fcurve.cpp
  floatfcurve.cpp
  abstractfcurve.cpp
  fcurveregister.cpp
)

file(GLOB HEADERS
  animator.h
  fcurve.h
  floatfcurve.h
  abstractfcurve.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

