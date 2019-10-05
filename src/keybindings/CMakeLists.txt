
file(GLOB SOURCES
  actionwidget.cpp
  action.cpp
  keybinding.cpp
  keybindings.cpp
  keybindingsdialog.cpp
  keybindingsproxymodel.cpp
  keybindingsview.cpp
  menu.cpp
)

file(GLOB HEADERS
  actionwidget.h
  action.h
  commandinterface.h
  keybinding.h
  keybindingsdialog.h
  keybindings.h
  keybindingsproxymodel.h
  keybindingsview.h
  menu.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

