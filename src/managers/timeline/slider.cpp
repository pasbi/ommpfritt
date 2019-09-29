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
  m_canvas.set_font(font());
  connect(&animator, SIGNAL(start_changed(int)), this, SLOT(update()));
  connect(&animator, SIGNAL(end_changed(int)), this, SLOT(update()));
  connect(&animator, SIGNAL(current_changed(int)), this, SLOT(update()));
  connect(&animator, SIGNAL(track_changed(Track&)), this, SLOT(update()));
  connect(&animator.scene.message_box(),
          SIGNAL(selection_changed(const std::set<AbstractPropertyOwner*>&)),
          this, SLOT(update()));
  connect(&m_canvas, SIGNAL(current_frame_changed(int)), this, SIGNAL(value_changed(int)));
}

void Slider::set_range(double left, double right)
{
  m_canvas.left_frame = left;
  m_canvas.right_frame = right;
}

void Slider::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::HighQualityAntialiasing);
  m_canvas.rect = rect();

  std::set<Track*> tracks;
  for (const AbstractPropertyOwner* o : m_scene.selection()) {
    for (Property* property : o->properties().values()) {
      if (Track* track = property->track(); track != nullptr) {
        tracks.insert(track);
      }
    }
  }
  m_canvas.tracks = tracks;
  painter.save();
  m_canvas.draw(painter);
  painter.restore();

  QWidget::paintEvent(event);
}

void Slider::mousePressEvent(QMouseEvent *event)
{
  m_canvas.mouse_press(*event);
  QWidget::mousePressEvent(event);
}

void Slider::mouseMoveEvent(QMouseEvent *event)
{
  m_canvas.mouse_move(*event);
  QWidget::mouseMoveEvent(event);
}

void Slider::mouseReleaseEvent(QMouseEvent* event)
{
  m_canvas.mouse_release(*event);
  QWidget::mouseReleaseEvent(event);
}

Slider::TimelineCanvasC::TimelineCanvasC(Animator& animator, Slider& self)
  : TimelineCanvas(animator), m_self(self)
{
}

QPoint Slider::TimelineCanvasC::map_to_global(const QPoint& pos) const
{
  return m_self.mapToGlobal(pos);
}

void Slider::TimelineCanvasC::update()
{
  m_self.update();
}

}  // namespace omm
