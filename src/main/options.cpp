#include "main/options.h"
#include <QRectF>

namespace omm
{
Options::Options(bool is_cli, bool have_opengl) : is_cli(is_cli), have_opengl(have_opengl)
{
}

Options::Anchor Options::anchor() const
{
  return m_anchor;
}

QPointF Options::anchor_position(const QRectF& grid, const Options::Anchor& anchor)
{
  switch (anchor) {
  case Anchor::TopLeft:
    return grid.topLeft();
  case Anchor::TopRight:
    return grid.topRight();
  case Anchor::BottomLeft:
    return grid.bottomLeft();
  case Anchor::BottomRight:
    return grid.bottomRight();
  case Anchor::Center:
    return grid.center();
  case Anchor::None:
  default:
    return QPointF(std::numeric_limits<double>::quiet_NaN(),
                   std::numeric_limits<double>::quiet_NaN());
  }
}

QPointF Options::anchor_position(const QRectF& grid) const
{
  return anchor_position(grid, anchor());
}

bool Options::require_gui() const
{
  return have_opengl;
}

void Options::set_anchor(Options::Anchor anchor)
{
  if (m_anchor != anchor) {
    m_anchor = anchor;
    Q_EMIT anchor_changed(anchor);
  }
}

}  // namespace omm
