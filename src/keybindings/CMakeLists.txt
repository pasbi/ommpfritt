
file(GLOB SOURCES
  action.cpp
  keybinding.cpp
  keybindings.cpp
)

file(GLOB HEADERS
  action.h
  commandinterface.h
  keybinding.h
  keybindingsdialog.h
  keybindings.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

