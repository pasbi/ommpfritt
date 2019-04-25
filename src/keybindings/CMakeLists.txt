file (GLOB SOURCES
  "action.cpp"
  "commandinterface.cpp"
  "defaultkeysequenceparser.cpp"
  "keybinding.cpp"
  "keybindings.cpp"
  "keybindingsdialog.cpp"
  "keybindingstable.cpp"
  "keybindingsproxymodel.cpp"
  "keybindingsdialogcontrols.cpp"
)
target_sources(libommpfritt PRIVATE ${SOURCES})
