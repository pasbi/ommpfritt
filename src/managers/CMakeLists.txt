target_sources(libommpfritt PRIVATE
  itemmanager.cpp
  itemmanager.h
  manager.cpp
  manager.h
  manageritemview.cpp
  manageritemview.h
  managertitlebar.cpp
  managertitlebar.h
  quickaccessdelegate.cpp
  quickaccessdelegate.h
  panzoomcontroller.cpp
  panzoomcontroller.h
  range.cpp
  range.h
)

add_subdirectory(boundingboxmanager)
add_subdirectory(curvemanager)
add_subdirectory(dopesheetmanager)
add_subdirectory(historymanager)
add_subdirectory(nodemanager)
add_subdirectory(objectmanager)
add_subdirectory(propertymanager)
add_subdirectory(pythonconsole)
add_subdirectory(stylemanager)
add_subdirectory(timeline)
