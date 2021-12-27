#include "geometry/rectangle.h"
#include "logging.h"

namespace omm
{
Rectangle::Rectangle() : m_is_empty(true)
{
}

Rectangle::Rectangle(const QRectF& rect) : Rectangle(rect.topLeft(), rect.bottomRight())
{
}
Rectangle::Rectangle(const Vec2f& top_left, const Vec2f& bottom_right, bool empty)
    : m_top_left(top_left), m_bottom_right(bottom_right), m_is_empty(empty)
{
}

Rectangle::Rectangle(const Vec2f& center, const double radius)
    : m_top_left(center - radius), m_bottom_right(center + radius), m_is_empty(false)
{
}

bool Rectangle::contains(const Vec2f& point) const
{
  return m_top_left.x <= point.x && point.x <= m_bottom_right.x && m_top_left.y <= point.y
         && point.y <= m_bottom_right.y;
}

bool Rectangle::is_empty() const
{
  return m_is_empty;
}

Vec2f Rectangle::top_left() const
{
  return m_top_left;
}

Vec2f Rectangle::top_right() const
{
  return {m_bottom_right.x, m_top_left.y};
}

Vec2f Rectangle::bottom_left() const
{
  return {m_top_left.x, m_bottom_right.y};
}

Vec2f Rectangle::bottom_right() const
{
  return m_bottom_right;
}

double Rectangle::left() const
{
  return top_left().x;
}

double Rectangle::top() const
{
  return top_left().y;
}

double Rectangle::right() const
{
  return bottom_right().x;
}

double Rectangle::bottom() const
{
  return bottom_right().y;
}

double Rectangle::width() const
{
  return right() - left();
}

double Rectangle::height() const
{
  return bottom() - top();
}

Vec2f Rectangle::size() const
{
  return {width(), height()};
}

Rectangle::operator QRectF() const
{
  return {m_top_left.to_pointf(), m_bottom_right.to_pointf()};
}

std::vector<Point> Rectangle::to_points() const
{
  return {Point(top_left()), Point(top_right()), Point(bottom_right()), Point(bottom_left())};
}

bool operator==(const Rectangle& a, const Rectangle& b)
{
  return a.top_left() == b.top_left() && a.bottom_right() == b.bottom_right();
}

}  // namespace omm
