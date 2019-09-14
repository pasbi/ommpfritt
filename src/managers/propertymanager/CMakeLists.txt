
file(GLOB SOURCES
  propertymanager.cpp
  propertymanagertab.cpp
)

file(GLOB HEADERS
  propertymanager.h
  propertymanagertab.h
)

add_subdirectory(userpropertymanager)
add_subdirectory(userpropertymanager_old)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

