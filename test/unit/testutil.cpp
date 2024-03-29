#include "testutil.h"
#include "registers.h"
#include "main/application.h"
#include "main/options.h"
#include <QApplication>
#include <QProcessEnvironment>
#include <QTimer>

namespace ommtest
{

Application::Application(std::unique_ptr<omm::Options> options)
    : m_q_application(argc, argv.data())
    , m_omm_application(std::make_unique<omm::Application>(m_q_application, std::move(options)))
{
}

Application::~Application() = default;

omm::Application& Application::omm_app() const
{
  return *m_omm_application;
}

bool have_opengl()
{
  const auto value = QProcessEnvironment::systemEnvironment().value("HAVE_OPENGL", "0");
  return value != "0";
}

}  // namespace ommtest
