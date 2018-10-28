file (GLOB SOURCES
  main.cpp
)

add_subdirectory(commands)
add_subdirectory(mainwindow)
add_subdirectory(managers)
add_subdirectory(objects)
add_subdirectory(properties)
add_subdirectory(python)
add_subdirectory(scene)
add_subdirectory(tags)

target_sources(ommpfritt PRIVATE ${SOURCES})
