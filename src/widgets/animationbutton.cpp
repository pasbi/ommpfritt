#include "widgets/animationbutton.h"

#include "scene/scene.h"
#include <QResizeEvent>
#include <QPainter>
#include <cmath>
#include <QMenu>
#include "logging.h"
#include "animation/animator.h"
#include "animation/track.h"
#include "aspects/propertyowner.h"
#include "commands/trackcommand.h"
#include "commands/keyframecommand.h"

namespace omm
{

AnimationButton::AnimationButton(Animator& animator, const std::set<Property*>& properties,
                                 QWidget *parent)
  : QWidget(parent)
  , m_animator(animator)
  , m_properties(properties)
{
  setContextMenuPolicy(Qt::DefaultContextMenu);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  connect(&animator, SIGNAL(current_changed(int)), this, SLOT(update()));
  connect(&animator, SIGNAL(tracks_changed()), this, SLOT(update()));
  for (const Property* property : m_properties) {
    connect(property, SIGNAL(value_changed(Property*)), this, SLOT(update()));
  }
}

bool AnimationButton::has_track() const
{
  return std::any_of(m_properties.begin(), m_properties.end(), [](auto* property) {
    return property->track() != nullptr;
  });
}

bool AnimationButton::value_is_inconsistent() const
{
  const int current_frame = m_animator.current();
  return std::any_of(m_properties.begin(), m_properties.end(), [current_frame](auto* property) {
    const Track* track = property->track();
    if (track == nullptr) {
      return false;  // there is no track, hence there is no inconsistency.
    } else {
      return !track->is_consistent(current_frame);
    }
  });
}

bool AnimationButton::has_key() const
{
  const int current_frame = m_animator.current();
  return std::any_of(m_properties.begin(), m_properties.end(), [current_frame](auto* property) {
    Track* track = property->track();
    return track != nullptr && track->has_keyframe(current_frame);
  });
}

void AnimationButton::set_key()
{
  {
    std::set<std::unique_ptr<Track>> new_tracks;
    for (Property* property : m_properties) {
      if (property->track() == nullptr) {
        new_tracks.insert(std::make_unique<Track>(*property));
      }
    }
    if (!new_tracks.empty()) {
      m_animator.scene.submit<InsertTracksCommand>(std::move(new_tracks));
    }
  }
  m_animator.scene.submit(std::make_unique<InsertKeyframeCommand>(m_animator.current(),
                                                                  m_properties));
  update();
}

void AnimationButton::remove_key()
{
  m_animator.scene.submit(std::make_unique<RemoveKeyframeCommand>(m_animator.current(),
                                                                  m_properties));
  update();
}

void AnimationButton::remove_track()
{
  m_animator.scene.submit(std::make_unique<RemoveTracksCommand>(m_properties));
}

void AnimationButton::resizeEvent(QResizeEvent *event)
{
  setFixedWidth(event->size().height());
  QWidget::resizeEvent(event);
}

void AnimationButton::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::HighQualityAntialiasing);
  QWidget::paintEvent(event);

  const QRectF rect = this->rect();
  auto centered = [rect](const double relative_radius) {
    const QSizeF absolute_radius = relative_radius * rect.size() / 2.0;
    const QPointF tl = rect.center() - QPointF(absolute_radius.width(), absolute_radius.height());
    return QRectF(tl, 2*absolute_radius);
  };
  const double pen_width_base = std::min(rect.width(), rect.height());

  QPen pen;
  if (!has_track()) {
    // no track, hence no key
    pen.setColor(Qt::black);
    pen.setWidthF(pen_width_base * 0.2);
    painter.setPen(pen);
    painter.drawEllipse(centered(0.8));
  } else if (!has_key()) {
    // track but no key
    pen.setColor(Qt::red);
    pen.setWidthF(pen_width_base * 0.2);
    painter.setPen(pen);
    painter.drawEllipse(centered(0.8));
  } else {
    // has key
    pen.setColor(Qt::red);
    pen.setWidthF(pen_width_base * 0.2);
    painter.setPen(pen);
    painter.drawEllipse(centered(0.8));
    QPainterPath ellipse;
    ellipse.addEllipse(centered(0.4));
    if (value_is_inconsistent()) {
      painter.fillPath(ellipse, QColor(255, 128, 0));
    } else {
      painter.fillPath(ellipse, Qt::red);
    }
  }
}

void AnimationButton::contextMenuEvent(QContextMenuEvent *event)
{
  auto context_menu = QMenu();

  auto make_action = [&](const QString& label, void(AnimationButton::*on_triggered)(), bool enabled)
  {
    QAction* action = context_menu.addAction(label);
    connect(action, &QAction::triggered, this, on_triggered);
    action->setEnabled(enabled);
  };

  make_action(tr("Remove Track"), &AnimationButton::remove_track, has_track());
  make_action(has_key() ? tr("Replace Key") : tr("Add Key"), &AnimationButton::set_key, true);
  make_action(tr("Remove Key"), &AnimationButton::remove_key, has_key());
  context_menu.exec(mapToGlobal(event->pos()));
}

void AnimationButton::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton) {
    set_key();
  }
}

}  // namespace omm
