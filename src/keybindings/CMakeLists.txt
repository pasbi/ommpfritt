
file(GLOB SOURCES
  actionwidget.cpp
  action.cpp
  keybinding.cpp
  keybindings.cpp
  keybindingsdialog.cpp
  keybindingsproxymodel.cpp
  keybindingsview.cpp
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
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

