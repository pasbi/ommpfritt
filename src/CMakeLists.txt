target_sources(libommpfritt PRIVATE
  ${CMAKE_BINARY_DIR}/config.cpp
  ${CMAKE_BINARY_DIR}/config.h
  abstractfactory.h
  cache.h
  cachedgetter.h
  common.cpp
  common.h
  disjointset.h
  dnf.cpp
  dnf.h
  enumnames.cpp
  enumnames.h
  logging.cpp
  logging.h
  maybeowner.h
  menuhelper.h
  orderedmap.h
  propertytypeenum.cpp
  propertytypeenum.h
  registers.cpp
  registers.h
  splinetype.cpp
  splinetype.h
  syncpalettedecorator.cpp
  syncpalettedecorator.h
  translator.cpp
  translator.h
  transparentset.h
  variant.cpp
  variant.h
)

add_subdirectory(animation)
add_subdirectory(aspects)
add_subdirectory(color)
add_subdirectory(commands)
add_subdirectory(geometry)
add_subdirectory(keybindings)
add_subdirectory(main)
add_subdirectory(mainwindow)
add_subdirectory(managers)
add_subdirectory(objects)
add_subdirectory(path)
add_subdirectory(preferences)
add_subdirectory(properties)
add_subdirectory(propertywidgets)
add_subdirectory(python)
add_subdirectory(renderers)
add_subdirectory(scene)
add_subdirectory(serializers)
add_subdirectory(tags)
add_subdirectory(tools)
add_subdirectory(widgets)
add_subdirectory(nodesystem)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_BINARY_DIR}/config.h")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.cpp.in" "${CMAKE_BINARY_DIR}/config.cpp")
