#include "widgets/animationbutton.h"

#include <QResizeEvent>
#include <QPainter>
#include <cmath>
#include <QMenu>
#include "logging.h"
#include "animation/animator.h"
#include "animation/track.h"
#include "aspects/propertyowner.h"

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
}

bool AnimationButton::has_track() const
{
  return std::any_of(m_owners.begin(), m_owners.end(), [this](auto* owner) {
    return m_animator.track(*owner, m_property_key) != nullptr;
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
  const int frame = m_animator.current();
  for (AbstractPropertyOwner* owner : m_owners) {
    Track* track = m_animator.track(*owner, m_property_key);
    if (track == nullptr) {
      track = m_animator.create_track(*owner, m_property_key);
    }
    if (track->has_keyframe(frame)) {
      track->remove_keyframe(frame);
    }
    track->record(frame, *owner->property(m_property_key));
  }
  update();
}

void AnimationButton::remove_key()
{
  const int frame = m_animator.current();
  for (AbstractPropertyOwner* owner : m_owners) {
    Track* track = m_animator.track(*owner, m_property_key);
    if (track != nullptr && track->has_keyframe(frame)) {
      track->remove_keyframe(frame);
    }
  }
  update();
}

void AnimationButton::remove_track()
{
  for (AbstractPropertyOwner* owner : m_owners) {
    Track* track = m_animator.track(*owner, m_property_key);
    if (track != nullptr) {
      m_animator.extract_track(*owner, m_property_key);
    }
  }
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
    painter.fillPath(ellipse, Qt::red);
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
