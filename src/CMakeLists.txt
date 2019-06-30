
file(GLOB SOURCES
  common.cpp
  logging.cpp
  main.cpp
)

file(GLOB HEADERS
  abstractfactory.h
  abstractraiiguard.h
  cache.h
  common.h
  logging.h
  maybeowner.h
  menuhelper.h
  observed.h
  orderedmap.h
)

add_subdirectory(aspects)
add_subdirectory(color)
add_subdirectory(commands)
add_subdirectory(external)
add_subdirectory(geometry)
add_subdirectory(keybindings)
add_subdirectory(mainwindow)
add_subdirectory(managers)
add_subdirectory(objects)
add_subdirectory(properties)
add_subdirectory(propertywidgets)
add_subdirectory(python)
add_subdirectory(renderers)
add_subdirectory(scene)
add_subdirectory(serializers)
add_subdirectory(tags)
add_subdirectory(tools)
add_subdirectory(widgets)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

