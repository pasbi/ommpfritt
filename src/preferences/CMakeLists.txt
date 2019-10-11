
file(GLOB SOURCES
  keybindingspage.cpp
  keybindingsproxymodel.cpp
  keybindingstreeviewdelegate.cpp
  preferencedialog.cpp
  preferencestree.cpp
  preferencestreeitem.cpp
  preferencestreeview.cpp
  preferencestreeviewdelegate.cpp
  uicolors.cpp
  uicolorspage.cpp
  uicolorstreeviewdelegate.cpp
)

file(GLOB HEADERS
  keybindingspage.h
  keybindingsproxymodel.h
  keybindingstreeviewdelegate.h
  preferencedialog.h
  preferencepage.h
  preferencestree.h
  preferencestreeitem.h
  preferencestreeview.h
  preferencestreeviewdelegate.h
  uicolors.h
  uicolorspage.h
  uicolorstreeviewdelegate.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

