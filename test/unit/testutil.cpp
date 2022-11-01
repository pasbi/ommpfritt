#include "testutil.h"
#include "main/application.h"
#include "main/options.h"
#include "path/dedge.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"
#include "registers.h"
#include "gtest/gtest.h"
#include <QApplication>
#include <QProcessEnvironment>
#include <QTimer>
#include <fmt/core.h>

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

QString Application::test_id_for_filename()
{
  QString name(::testing::UnitTest::GetInstance()->current_test_case()->name());
  return name.replace("/", "_");
}

bool have_opengl()
{
  const auto value = QProcessEnvironment::systemEnvironment().value("HAVE_OPENGL", "0");
  return value != "0";
}

omm::Point EllipseMaker::ith_point(const std::size_t i) const
{
  const double theta = M_PI * 2.0 * static_cast<double>(i) / static_cast<double>(point_count);
  const auto pos = omm::Vec2f(radii.x * std::cos(theta), radii.y * std::sin(theta)) + origin;
  if (no_tangents) {
    return omm::Point(pos);
  } else {
    const auto t_scale = 4.0 / 3.0 * std::tan(M_PI / (2.0 * static_cast<double>(point_count)));
    const auto t = omm::Vec2f(-std::sin(theta), std::cos(theta)) * static_cast<double>(t_scale);
    return omm::Point(pos, omm::PolarCoordinates(-t), omm::PolarCoordinates(t));
  }
}

omm::Path& EllipseMaker::make_path(omm::PathVector& pv)
{
  assert(edges.empty());
  auto& path = pv.add_path();
  path.set_single_point(std::make_shared<omm::PathPoint>(ith_point(0), &pv));
  for (std::size_t i = 1; i < point_count; ++i) {
    auto new_point = std::make_shared<omm::PathPoint>(ith_point(i), &pv);
    edges.emplace_back(&path.add_edge(path.last_point(), std::move(new_point)), omm::Direction::Forward);
  }
  if (closed && point_count > 1) {
    edges.emplace_back(&path.add_edge(path.last_point(), path.first_point()), omm::Direction::Forward);
  }
  return path;
}

std::string EllipseMaker::to_string() const
{
  const auto s_open_closed = closed ? "closed" : "open";
  const auto s_interp = no_tangents ? "linear" : "smooth";
  return fmt::format("{}-Ellipse-{}-{}", point_count, s_open_closed, s_interp);
}

std::set<std::deque<omm::DEdge>> EllipseMaker::faces() const
{
  if (closed) {
    return {edges};
  }
  return {};
}

double EllipseMaker::area() const
{
  return M_PI * radii.x * radii.y;
}

std::list<std::unique_ptr<omm::PathVector>> PathVectorHeap::m_path_vectors;

omm::PathVector* PathVectorHeap::annex(std::unique_ptr<omm::PathVector> pv)
{
  return m_path_vectors.emplace_back(std::move(pv)).get();
}

}  // namespace ommtest
