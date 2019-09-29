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

  painter.save();
  m_canvas.draw_background(painter);
  m_canvas.draw_lines(painter);
  m_canvas.draw_keyframes(painter);
  m_canvas.draw_current(painter);
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

void Slider::keyPressEvent(QKeyEvent* event)
{
  m_canvas.key_press(*event);
}

Slider::TimelineCanvasC::TimelineCanvasC(Animator& animator, Slider& self)
  : TimelineCanvas(animator, QFontMetrics(self.font()).height()), m_self(self)
{
  connect(&animator.scene.message_box(),
          qOverload<const std::set<AbstractPropertyOwner*>&>(&MessageBox::selection_changed),
          this, &TimelineCanvasC::update_tracks);
  update_tracks(animator.scene.selection());
  connect(&animator, &Animator::track_inserted, this, [this](Track& track) {
    m_self.m_canvas.tracks.insert(&track);
    m_self.m_canvas.update();
  });
  connect(&animator, &Animator::track_removed, this, [this](Track& track) {
    m_self.m_canvas.tracks.erase(&track);
    m_self.m_canvas.update();
  });
}

QPoint Slider::TimelineCanvasC::map_to_global(const QPoint& pos) const
{
  return m_self.mapToGlobal(pos);
}

void Slider::TimelineCanvasC::update()
{
  m_self.update();
}

void Slider::TimelineCanvasC::update_tracks(const std::set<AbstractPropertyOwner*>& selection)
{
  tracks.clear();
  for (AbstractPropertyOwner* apo : selection) {
    for (Property* p : apo->properties().values()) {
      if (Track* track = p->track(); track != nullptr) {
        tracks.insert(track);
      }
    }
  }
  update();
}

}  // namespace omm
