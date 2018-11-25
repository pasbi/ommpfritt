file (GLOB SOURCES
  main.cpp
)

target_sources(ommpfritt PRIVATE ${SOURCES})

add_subdirectory(aspects)
add_subdirectory(color)
add_subdirectory(commands)
add_subdirectory(geometry)
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
