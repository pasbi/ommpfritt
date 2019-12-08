#include "managers/nodemanager/panzoomcontroller.h"
#include <cmath>

namespace omm
{

void PanZoomController::start(PanZoomController::Action action)
{
  m_current_action = action;
  m_start_pos = m_last_pos;
}

void PanZoomController::end()
{
  m_current_action = Action::None;
}

bool PanZoomController::move(const QPoint& pos)
{
  m_d = pos - m_last_pos;
  bool invertible;
  const QTransform ti = m_transform.inverted(&invertible);
  assert(invertible);
  m_e = ti.map(pos) - ti.map(m_last_pos);
  m_last_pos = pos;
  switch (m_current_action) {
  case Action::Pan:
    pan();
    return true;
  case Action::Zoom:
    zoom();
    return true;
  default:
    return false;
  }
}

void PanZoomController::pan()
{
  QTransform t;
  t.translate(m_d.x(), m_d.y());
  m_transform  = m_transform * t;
}

void PanZoomController::zoom()
{
  const double sign = std::abs(m_d.x()) > std::abs(m_d.y()) ? m_d.x() : m_d.y();
  const double d2 = std::copysign(std::sqrt(QPointF::dotProduct(m_d, m_d)), sign);
  const double s = std::exp(d2 / 600.0);

  QTransform tt;
  tt.translate(-m_start_pos.x(), -m_start_pos.y());

  QTransform ts;
  ts.scale(s, s);

  QTransform tti;
  tti.translate(m_start_pos.x(), m_start_pos.y());

  m_transform = m_transform * tt * ts * tti;
}

}  // namespace
