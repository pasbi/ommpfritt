
file(GLOB SOURCES
  propertymanager.cpp
  propertymanagertab.cpp
  propertymanagertitlebar.cpp
)

file(GLOB HEADERS
  propertymanager.h
  propertymanagertab.h
  propertymanagertitlebar.h
)

add_subdirectory(userpropertymanager)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

