
file(GLOB SOURCES
  common.cpp
  enumnames.cpp
  dnf.cpp
  logging.cpp
  proxychain.cpp
  splinetype.cpp
  subcommandlineparser.cpp
  variant.cpp
  registers.cpp
)

file(GLOB HEADERS
  abstractfactory.h
  cache.h
  cachedgetter.h
  common.h
  enumnames.h
  dnf.h
  logging.h
  maybeowner.h
  menuhelper.h
  orderedmap.h
  variant.h
  proxychain.h
  splinetype.h
  subcommandlineparser.h
  translations.h
  register.h
)

add_subdirectory(animation)
add_subdirectory(aspects)
add_subdirectory(color)
add_subdirectory(commands)
add_subdirectory(external)
add_subdirectory(geometry)
add_subdirectory(keybindings)
add_subdirectory(mainwindow)
add_subdirectory(managers)
add_subdirectory(objects)
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

file(GLOB APP_SOURCES main.cpp)
file(GLOB CLI_SOURCES maincli.cpp)

target_sources(ommpfritt PUBLIC ${APP_SOURCES})
target_sources(ommpfritt-cli PUBLIC ${CLI_SOURCES})
target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

