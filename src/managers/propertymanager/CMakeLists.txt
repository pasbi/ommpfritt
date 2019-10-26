
file(GLOB SOURCES
  propertymanager.cpp
  propertymanagertab.cpp
  propertymanagertitlebar.cpp
  userpropertydialog.cpp
  userpropertylistmodel.cpp
)

file(GLOB HEADERS
  propertymanager.h
  propertymanagertab.h
  propertymanagertitlebar.h
  userpropertydialog.h
  userpropertylistmodel.h
)

target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

