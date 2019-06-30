
file(GLOB SOURCES
  brushselecttool.cpp
  knifetool.cpp
  pathtool.cpp
  selectobjectstool.cpp
  selectpointstool.cpp
  selecttool.cpp
  toolbox.cpp
  tool.cpp
  toolregister.cpp
)

file(GLOB HEADERS
  brushselecttool.h
  knifetool.h
  pathtool.h
  selectobjectstool.h
  selectpointstool.h
  selecttool.h
  toolbox.h
  tool.h
)

add_subdirectory(handles)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

