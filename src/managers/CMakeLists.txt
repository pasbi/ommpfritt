
file(GLOB SOURCES
  itemmanager.cpp
  manager.cpp
  manageritemview.cpp
  managerregister.cpp
  managertitlebar.cpp
  quickaccessdelegate.cpp
  range.cpp
)

file(GLOB HEADERS
  itemmanager.h
  manager.h
  manageritemview.h
  managertitlebar.h
  quickaccessdelegate.h
  range.h
)

add_subdirectory(boundingboxmanager)
add_subdirectory(curvemanager)
add_subdirectory(dopesheet)
add_subdirectory(historymanager)
add_subdirectory(objectmanager)
add_subdirectory(propertymanager)
add_subdirectory(pythonconsole)
add_subdirectory(stylemanager)
add_subdirectory(timeline)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

