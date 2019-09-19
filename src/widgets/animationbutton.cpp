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

AnimationButton::AnimationButton(Animator& animator, const std::set<AbstractPropertyOwner*>& owners,
                                 const std::string& property_key, QWidget *parent)
  : QWidget(parent)
  , m_animator(animator)
  , m_owners(owners)
  , m_property_key(property_key)
{
  setContextMenuPolicy(Qt::DefaultContextMenu);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  connect(&animator, SIGNAL(current_changed(int)), this, SLOT(update()));
  connect(&animator, SIGNAL(tracks_changed()), this, SLOT(update()));
  for (const AbstractPropertyOwner* owner : m_owners) {
    connect(owner->property(m_property_key), SIGNAL(value_changed(Property*)),
            this, SLOT(update()));
  }
}

bool AnimationButton::has_track() const
{
  return std::any_of(m_owners.begin(), m_owners.end(), [this](auto* owner) {
    return m_animator.track(*owner, m_property_key) != nullptr;
  });
}

bool AnimationButton::value_coincides() const
{
  return std::all_of(m_owners.begin(), m_owners.end(), [this](auto* owner) {
    const Track* track = m_animator.track(*owner, m_property_key);
    const Property& property = track->property();
    return property.variant_value() == track->interpolate(m_animator.current());
  });
}

bool AnimationButton::has_key() const
{
  const int current_frame = m_animator.current();
  return std::any_of(m_owners.begin(), m_owners.end(), [this, current_frame](auto* owner) {
    Track* track = m_animator.track(*owner, m_property_key);
    return track != nullptr && track->has_keyframe(current_frame);
  });
}

void AnimationButton::set_key()
{
  {
    std::set<std::unique_ptr<Track>> new_tracks;
    for (AbstractPropertyOwner* owner : m_owners) {
      Track* track = m_animator.track(*owner, m_property_key);
      if (track == nullptr) {
        new_tracks.insert(std::make_unique<Track>(*owner, m_property_key));
      }
    }
    if (!new_tracks.empty()) {
      m_animator.scene.submit<InsertTracksCommand>(m_animator, std::move(new_tracks));
    }
  }
  m_animator.scene.submit(std::make_unique<InsertKeyframeCommand>(m_animator, m_animator.current(),
                                                                  m_owners, m_property_key));
  update();
}

void AnimationButton::remove_key()
{
  m_animator.scene.submit(std::make_unique<RemoveKeyframeCommand>(m_animator, m_animator.current(),
                                                                  m_owners, m_property_key));
  update();
}

void AnimationButton::remove_track()
{
  m_animator.scene.submit(std::make_unique<RemoveTracksCommand>(m_animator,
                                                                m_owners, m_property_key));
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
    if (value_coincides()) {
      painter.fillPath(ellipse, Qt::red);
    } else {
      painter.fillPath(ellipse, QColor(255, 128, 0));
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
