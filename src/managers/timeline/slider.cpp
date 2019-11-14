#include "managers/timeline/slider.h"
#include "logging.h"
#include <QMouseEvent>
#include <QPainter>
#include <cmath>
#include "animation/animator.h"
#include "scene/scene.h"
#include "scene/messagebox.h"
#include "managers/dopesheet/trackviewdelegate.h"

namespace omm
{

Slider::Slider(Animator& animator)
  : m_canvas(animator, *this)
  , m_scene(animator.scene)
{
  connect(&m_canvas, SIGNAL(current_frame_changed(int)), this, SIGNAL(value_changed(int)));
  connect(&animator, SIGNAL(current_changed(int)), this, SLOT(update()));

  m_canvas.footer_height = QFontMetrics(font()).height();
}

void Slider::set_range(double left, double right)
{
  m_canvas.frame_range.begin = left;
  m_canvas.frame_range.end = right;
}

void Slider::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::HighQualityAntialiasing);
  m_canvas.rect = rect();

  painter.save();
  m_canvas.draw_background(painter);
  m_canvas.draw_lines(painter);
  m_canvas.draw_keyframes(painter);
  m_canvas.draw_current(painter);
  m_canvas.draw_rubber_band(painter);
  painter.restore();

  QWidget::paintEvent(event);
}

std::set<Track*> Slider::tracks() const
{
  return m_canvas.tracks;
}

bool Slider::event(QEvent* event)
{
  if (m_canvas.view_event(*event)) {
    event->accept();
    return true;
  } else {
    return QWidget::event(event);
  }
}

Slider::TimelineCanvasC::TimelineCanvasC(Animator& animator, Slider& self)
  : TimelineCanvas(animator, self), m_self(self)
{
  synchronize_track_selection_with_animator();
  update_tracks(animator.scene.selection());
}

QPoint Slider::TimelineCanvasC::map_to_global(const QPoint& pos) const
{
  return m_self.mapToGlobal(pos);
}

void Slider::TimelineCanvasC::update()
{
  m_self.update();
}

void Slider::TimelineCanvasC::disable_context_menu()
{
  m_self.setContextMenuPolicy(Qt::PreventContextMenu);
}

void Slider::TimelineCanvasC::enable_context_menu()
{
  m_self.setContextMenuPolicy(Qt::DefaultContextMenu);
}

QRect Slider::TimelineCanvasC::track_rect(Track& track)
{
  if (::contains(tracks, &track)) {
    return m_self.rect();
  } else {
    return QRect();
  }
}

QRect Slider::TimelineCanvasC::owner_rect(AbstractPropertyOwner& owner)
{
  // this view never shows owner.
  Q_UNUSED(owner)
  return QRect();
}

}  // namespace omm
