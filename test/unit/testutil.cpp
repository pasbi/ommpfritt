#include "testutil.h"
#include "registers.h"

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

}  // namespace ommtest
