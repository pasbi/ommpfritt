
file(GLOB SOURCES
  action.cpp
  commandinterface.cpp
  defaultkeysequenceparser.cpp
  keybinding.cpp
  keybindings.cpp
  keybindingsdialogcontrols.cpp
  keybindingsdialog.cpp
  keybindingsproxymodel.cpp
  keybindingstable.cpp
)

file(GLOB HEADERS
  action.h
  commandinterface.h
  defaultkeysequenceparser.h
  keybinding.h
  keybindingsdialogcontrols.h
  keybindingsdialog.h
  keybindings.h
  keybindingsproxymodel.h
  keybindingstable.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

