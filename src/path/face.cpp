#include "path/face.h"
#include "path/edge.h"
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

bool Face::contains(const Face& other) const
{
  // this and other must be `simply_closed`, i.e. not intersect themselves respectively.
  assert(is_valid());
  assert(other.is_valid());
  auto pvv_a = path_vector_view();
  auto pvv_b = other.path_vector_view();
  pvv_a.normalize();
  pvv_b.normalize();

  std::size_t edge_index_a = 0;
  std::size_t edge_index_b = 0;
  while (true) {
  }
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

void Face::normalize()
{
  m_path_vector_view->normalize();
}

}  // namespace omm
