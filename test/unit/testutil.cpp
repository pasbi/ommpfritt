#include "testutil.h"
#include "registers.h"
#include "main/application.h"
#include "main/options.h"
#include <QApplication>
#include <QProcessEnvironment>
#include <QTimer>

namespace ommtest
{

GuiApplication::GuiApplication()
    : m_application(argc, argv.data())
{
}

std::unique_ptr<omm::Application> GuiApplication::make_application(std::unique_ptr<omm::Options> options)
{
  return std::make_unique<omm::Application>(m_application, std::move(options));
}


bool have_opengl()
{
  const auto value = QProcessEnvironment::systemEnvironment().value("HAVE_OPENGL", "0");
  return value != "0";
}

}  // namespace ommtest
