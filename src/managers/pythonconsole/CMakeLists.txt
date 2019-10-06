
file(GLOB SOURCES
  pythonconsole.cpp
  pythonconsoletitlebar.cpp
)

file(GLOB HEADERS
  pythonconsole.h
  pythonconsoletitlebar.h
)


target_sources(libommpfritt PRIVATE ${SOURCES} ${HEADERS})

