
file(GLOB SOURCES
  itemmanager.cpp
  manager.cpp
  manageritemview.cpp
  managerregister.cpp
)

file(GLOB HEADERS
  itemmanager.h
  manager.h
  manageritemview.h
)

add_subdirectory(boundingboxmanager)
add_subdirectory(historymanager)
add_subdirectory(objectmanager)
add_subdirectory(propertymanager)
add_subdirectory(pythonconsole)
add_subdirectory(stylemanager)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

