
file(GLOB SOURCES
  propertyitem.cpp
  userpropertydialog.cpp
)

file(GLOB HEADERS
  propertyitem.h
  userpropertydialog.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

