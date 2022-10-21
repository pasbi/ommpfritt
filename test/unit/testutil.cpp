#include "testutil.h"
#include "main/application.h"
#include "main/options.h"
#include "path/pathvector.h"
#include "registers.h"
#include <QApplication>
#include <QProcessEnvironment>
#include <QTimer>

namespace
{

std::unique_ptr<omm::Options> options()
{
  return std::make_unique<omm::Options>(false, // is_cli
                                        false  // have_opengl
  );
}

}  // namespace

namespace ommtest
{

Application::Application()
    : m_q_application(argc, argv.data())
    , m_omm_application(std::make_unique<omm::Application>(m_q_application, options()))
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

std::list<std::unique_ptr<omm::PathVector>> PathVectorHeap::m_path_vectors;

omm::PathVector* PathVectorHeap::annex(std::unique_ptr<omm::PathVector> pv)
{
  return m_path_vectors.emplace_back(std::move(pv)).get();
}

}  // namespace ommtest
