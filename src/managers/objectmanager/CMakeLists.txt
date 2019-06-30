
file(GLOB SOURCES
  objectmanager.cpp
  objectquickaccessdelegate.cpp
  objecttreeselectionmodel.cpp
  objecttreeview.cpp
  tagsitemdelegate.cpp
)

file(GLOB HEADERS
  objectmanager.h
  objectquickaccessdelegate.h
  objecttreeselectionmodel.h
  objecttreeview.h
  tagsitemdelegate.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

