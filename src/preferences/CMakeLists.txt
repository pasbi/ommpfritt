
file(GLOB SOURCES
  generalpage.cpp
  keybindingspage.cpp
  keybindingsproxymodel.cpp
  keybindingstreeviewdelegate.cpp
  keysequenceedit.cpp
  preferences.cpp
  preferencedialog.cpp
  preferencestree.cpp
  preferencestreeitem.cpp
  preferencestreeview.cpp
  preferencestreeviewdelegate.cpp
  uicoloredit.cpp
  uicolors.cpp
  uicolorspage.cpp
  uicolorstreeviewdelegate.cpp
)

file(GLOB HEADERS
  generalpage.h
  keybindingspage.h
  keybindingsproxymodel.h
  keybindingstreeviewdelegate.h
  keysequenceedit.h
  preferences.h
  preferencedialog.h
  preferencepage.h
  preferencestree.h
  preferencestreeitem.h
  preferencestreeview.h
  preferencestreeviewdelegate.h
  uicoloredit.h
  uicolors.h
  uicolorspage.h
  uicolorstreeviewdelegate.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

