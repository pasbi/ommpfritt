#include "path/face.h"
#include "geometry/line.h"
#include "path/dedge.h"
#include "path/pathpoint.h"
#include "path/pathvectorview.h"
#include <QPainterPath>
#include <QStringList>

namespace omm
{

Face::Face()
    : m_path_vector_view(std::make_unique<PathVectorView>())
{
}

Face::Face(PathVectorView pvv)
    : m_path_vector_view(std::make_unique<PathVectorView>(std::move(pvv)))
{
}

Face::Face(const Face& other)
    : m_path_vector_view(std::make_unique<PathVectorView>(other.path_vector_view()))
{
}

Face::Face(Face&& other) noexcept
    : Face()
{
  swap(*this, other);
}

Face& Face::operator=(Face other)
{
  swap(*this, other);
  return *this;
}

Face& Face::operator=(Face&& other) noexcept
{
  swap(*this, other);
  return *this;
}

Face::~Face() = default;

void swap(Face& a, Face& b) noexcept
{
  swap(a.m_path_vector_view, b.m_path_vector_view);
}

double Face::compute_aabb_area() const
{
  if (path_vector_view().edges().size()) {
    return 0.0;
  }

  double left = std::numeric_limits<double>::infinity();
  double right = -std::numeric_limits<double>::infinity();
  double top = -std::numeric_limits<double>::infinity();
  double bottom = std::numeric_limits<double>::infinity();

  for (const auto* pp : path_vector_view().path_points()) {
    const auto& p = pp->geometry();
    left = std::min(left, p.position().x);
    right = std::max(right, p.position().x);
    top = std::max(top, p.position().y);
    bottom = std::min(bottom, p.position().y);
  }

  return (right - left) * (top - bottom);
}

QString Face::to_string() const
{
  return m_path_vector_view->to_string();
}

bool Face::is_valid() const noexcept
{
  return m_path_vector_view->is_valid() && m_path_vector_view->is_simply_closed();
}

PathVectorView& Face::path_vector_view()
{
  return *m_path_vector_view;
}

const PathVectorView& Face::path_vector_view() const
{
  return *m_path_vector_view;
}

double Face::area() const
{
  // See Green's Theorem and Leibniz' Sektorformel
  const auto g_path = m_path_vector_view->to_geom();
  double sum = 0.0;
  for (std::size_t i = 0; i < g_path.size(); ++i) {
    const auto& curve = static_cast<const Geom::BezierCurveN<3>&>(g_path.at(i));
    const auto& derivative = static_cast<const Geom::BezierCurveN<2>&>(*curve.derivative());
    const auto x = curve.fragment()[0];
    const auto y = curve.fragment()[1];
    const auto dx = derivative.fragment()[0];
    const auto dy = derivative.fragment()[1];
    const auto integral = Geom::integral(y * dx + x * dy);
    sum += integral.valueAt(1.0) - integral.valueAt(0.0);
  }
  return sum / 2.0;
}

bool Face::operator==(const Face& other) const
{
  return *m_path_vector_view == other.path_vector_view();
}

bool Face::operator!=(const Face& other) const
{
  return !(*this == other);
}

bool Face::operator<(const Face& other) const
{
  return *m_path_vector_view < other.path_vector_view();
}

PolygonLocation polygon_contains(const std::vector<Vec2f>& polygon, const Vec2f& p)
{
  // Ray casting
  bool inside = false;
  const Line ray{p, p + Vec2(1.0, 0.0)};  // Ray with arbitary direction from p
  for (std::size_t i = 0; i < polygon.size(); ++i) {
    const Line line{i == 0 ? polygon.back() : polygon.at(i - 1), polygon.at(i)};
    static constexpr auto eps = 0.0001;
    static constexpr auto in01 = [](const double d) { return d >= 0.0 && d < 1.0; };
    if (std::abs(line.distance(p)) < eps && in01(line.project(p))) {
      return PolygonLocation::Edge;
    }
    const auto t = line.intersect(ray);
    const auto u = ray.intersect(line);
    if (!std::isfinite(t) || !std::isfinite(u)) {
      // line is parallel to ray and p is not on line: no intersection.
    } else if (u >= 0.0 && in01(t)) {
      // intersection
      inside = !inside;
    }
  }
  return inside ? PolygonLocation::Inside : PolygonLocation::Outside;
}

std::ostream& operator<<(std::ostream& os, const Face& face)
{
  return os << face.to_string().toStdString();
}

}  // namespace omm
