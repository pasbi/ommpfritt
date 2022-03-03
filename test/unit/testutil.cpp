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

std::unique_ptr<omm::Application> get_application(int kill_after_msec)
{
  auto app = std::make_unique<ommtest::GuiApplication>();

  auto options = std::make_unique<omm::Options>(false, // is_cli
                                                false  // have_opengl
  );
  auto omm_app = std::make_unique<omm::Application>(app->gui_application(), std::move(options));

  QTimer::singleShot(kill_after_msec, omm_app.get(), [app=app.release()] {
    QApplication::quit();
    delete app;
  });
  return omm_app;
}

}  // namespace ommtest
