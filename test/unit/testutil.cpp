#include "testutil.h"
#include "registers.h"
#include <QProcessEnvironment>

namespace ommtest
{

GuiApplication::GuiApplication()
    : m_application(argc, argv.data())
{
  omm::register_everything();
}

QGuiApplication& GuiApplication::gui_application()
{
  return m_application;
}

bool have_opengl()
{
  const auto value = QProcessEnvironment::systemEnvironment().value("HAVE_OPENGL", "0");
  return value != "0";
}

}  // namespace ommtest
