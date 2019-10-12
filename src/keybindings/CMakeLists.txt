
file(GLOB SOURCES
  actionwidget.cpp
  action.cpp
  keybinding.cpp
  keybindings.cpp
  menu.cpp
)

file(GLOB HEADERS
  actionwidget.h
  action.h
  commandinterface.h
  keybinding.h
  keybindingsdialog.h
  keybindings.h
  menu.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

