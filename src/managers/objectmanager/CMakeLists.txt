
file(GLOB SOURCES
  objectdelegate.cpp
  objectmanager.cpp
  objectquickaccessdelegate.cpp
  objecttreeselectionmodel.cpp
  objecttreeview.cpp
  tagsitemdelegate.cpp
)

file(GLOB HEADERS
  objectdelegate.h
  objectmanager.h
  objectquickaccessdelegate.h
  objecttreeselectionmodel.h
  objecttreeview.h
  tagsitemdelegate.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

