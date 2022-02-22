#include "widgets/animationbutton.h"

#include "animation/animator.h"
#include "animation/track.h"
#include "animation/knot.h"
#include "aspects/propertyowner.h"
#include "commands/keyframecommand.h"
#include "commands/setinterpolationcommand.h"
#include "commands/trackcommand.h"
#include "logging.h"
#include "preferences/uicolors.h"
#include "scene/history/historymodel.h"
#include "scene/history/macro.h"
#include "scene/scene.h"
#include "removeif.h"
#include <QMenu>
#include <QPainter>
#include <QResizeEvent>
#include <cmath>

namespace
{
template<typename K, typename V> std::set<V> values(const std::map<K, V>& map)
{
  std::set<V> vs;
  for (auto&& [k, v] : map) {
    vs.insert(v);
  }
  return vs;
}

}  // namespace

namespace omm
{
AnimationButton::AnimationButton(Animator& animator,
                                 const std::map<AbstractPropertyOwner*, Property*>& properties,
                                 QWidget* parent)
    : QWidget(parent), m_animator(animator), m_properties(properties)
{
  setContextMenuPolicy(Qt::DefaultContextMenu);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  connect(&animator, &Animator::current_changed, this, qOverload<>(&AnimationButton::update));
  connect(&animator, &Animator::track_changed, this, [this](Track&) {
    // update is very cheap and checking whether the track belongs to the butto is expensive.
    // since update never hurts, update even if this is track does not belong to the button.
    update();
  });
  for (auto&& [owner, property] : m_properties) {
    Q_UNUSED(owner)
    connect(property, &Property::value_changed, this, qOverload<>(&AnimationButton::update));
  }
}

bool AnimationButton::has_track() const
{
  return std::any_of(m_properties.begin(), m_properties.end(), [](auto&& arg) {
    return arg.second->track() != nullptr;
  });
}

bool AnimationButton::value_is_inconsistent() const
{
  const int current_frame = m_animator.current();
  return std::any_of(m_properties.begin(), m_properties.end(), [current_frame](auto&& arg) {
    const Track* track = arg.second->track();
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
  return std::any_of(m_properties.begin(), m_properties.end(), [current_frame](auto&& arg) {
    Track* track = arg.second->track();
    return track != nullptr && track->has_keyframe(current_frame);
  });
}

void AnimationButton::set_key()
{
  auto macro = m_animator.scene.history().start_macro(tr("Set keyframe"));
  {
    std::map<AbstractPropertyOwner*, std::unique_ptr<Track>> new_tracks;
    for (auto&& [owner, property] : m_properties) {
      if (property->track() == nullptr) {
        new_tracks.insert(std::pair(owner, std::make_unique<Track>(*property)));
      }
    }
    if (!new_tracks.empty()) {
      m_animator.scene.submit<InsertTracksCommand>(m_animator, std::move(new_tracks));
    }
  }

  const int current_time = m_animator.current();
  const std::set<Property*> p1 = values(m_properties);
  std::set<Property*> p2 = util::remove_if(p1, [current_time](Property* p) {
    return p->track() == nullptr || !p->track()->has_keyframe(current_time);
  });

  m_animator.scene.submit<RemoveKeyFrameCommand>(m_animator, current_time, p2);
  m_animator.scene.submit<InsertKeyFrameCommand>(m_animator, current_time, p1);
  update();
}

void AnimationButton::remove_key()
{
  m_animator.scene.submit<RemoveKeyFrameCommand>(m_animator,
                                                 m_animator.current(),
                                                 values(m_properties));
  update();
}

void AnimationButton::remove_track()
{
  m_animator.scene.submit(std::make_unique<RemoveTracksCommand>(m_animator, m_properties));
}

void AnimationButton::resizeEvent(QResizeEvent* event)
{
  setFixedWidth(event->size().height());
  QWidget::resizeEvent(event);
}

void AnimationButton::paintEvent(QPaintEvent* event)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  QWidget::paintEvent(event);

  const QRectF rect = this->rect();
  auto centered = [rect](const double relative_radius) {
    const QSizeF absolute_radius = relative_radius * rect.size() / 3.0;
    const QPointF tl = rect.center() - QPointF(absolute_radius.width(), absolute_radius.height());
    return QRectF(tl, 2 * absolute_radius);
  };
  const double pen_width_base = std::min(rect.width(), rect.height());

  static constexpr double PEN_WIDTH_MULTIPLIER = 0.1;
  static constexpr double ELLIPSE_RADIUS_LARGE = 0.8;
  static constexpr double ELLIPSE_RADIUS_MEDIUM = 0.4;
  static constexpr double ELLIPSE_RADIUS_SMALL = 0.3;

  QPen pen;
  if (!has_track()) {
    // no track, hence no key
    pen.setColor(ui_color(*this, "Preferences", "record normal"));
    pen.setWidthF(pen_width_base * PEN_WIDTH_MULTIPLIER);
    painter.setPen(pen);
    painter.drawEllipse(centered(ELLIPSE_RADIUS_SMALL));
  } else if (!has_key()) {
    // track but no key
    pen.setColor(ui_color(*this, "Preferences", "record keyframe"));
    pen.setWidthF(pen_width_base * PEN_WIDTH_MULTIPLIER);
    painter.setPen(pen);
    painter.drawEllipse(centered(ELLIPSE_RADIUS_LARGE));
  } else {
    // has key
    pen.setColor(ui_color(*this, "Preferences", "record keyframe"));
    pen.setWidthF(pen_width_base * PEN_WIDTH_MULTIPLIER);
    painter.setPen(pen);
    painter.drawEllipse(centered(ELLIPSE_RADIUS_LARGE));
    QPainterPath ellipse;
    ellipse.addEllipse(centered(ELLIPSE_RADIUS_MEDIUM));
    if (value_is_inconsistent()) {
      painter.fillPath(ellipse, ui_color(*this, "Preferences", "record inconsistent"));
    } else {
      painter.fillPath(ellipse, ui_color(*this, "Preferences", "record keyframe"));
    }
  }
}

void AnimationButton::contextMenuEvent(QContextMenuEvent* event)
{
  auto context_menu = QMenu();

  auto make_action
      = [&](const QString& label, void (AnimationButton::*on_triggered)(), bool enabled) {
          QAction* action = context_menu.addAction(label);
          connect(action, &QAction::triggered, this, on_triggered);
          action->setEnabled(enabled);
        };

  make_action(tr("Remove Track"), &AnimationButton::remove_track, has_track());
  make_action(has_key() ? tr("Replace Key") : tr("Add Key"), &AnimationButton::set_key, true);
  make_action(tr("Remove Key"), &AnimationButton::remove_key, has_key());

  context_menu.addMenu(make_interpolation_menu().release());

  context_menu.exec(mapToGlobal(event->pos()));
}

void AnimationButton::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton) {
    set_key();
  }
}

std::unique_ptr<QMenu> AnimationButton::make_interpolation_menu() const
{
  auto menu = std::make_unique<QMenu>(tr("Interpolation"));

  const bool is_numeric = std::all_of(m_properties.begin(), m_properties.end(), [](const auto& op) {
    const Property* property = op.second;
    return property->track() != nullptr && property->n_channels() > 0;
  });

  const std::map<Track::Interpolation, bool> options{
      {Track::Interpolation::Step, true},
      {Track::Interpolation::Bezier, is_numeric},
      {Track::Interpolation::Linear, is_numeric},
  };

  std::map<Track::Interpolation, QAction*> actions;

  for (auto&& [interpolation, enabled] : options) {
    const QString label = Track::interpolation_label(interpolation);
    QAction* action = menu->addAction(label);
    actions[interpolation] = action;
    action->setCheckable(true);
    if (enabled) {
      connect(action, &QAction::triggered, this, [this, interpolation = interpolation]() {
        std::set<Property*> properties;
        for (auto [owner, property] : m_properties) {
          if (property->track() != nullptr) {
            properties.insert(property);
          }
        }
        m_animator.scene.submit<SetInterpolationCommand>(properties, interpolation);
      });
    } else {
      action->setEnabled(false);
    }
  }

  for (auto&& [interpolation, enabled] : options) {
    bool all = true;
    for (auto [owner, property] : m_properties) {
      if (Track* track = property->track(); track != nullptr) {
        if (track->interpolation() != interpolation) {
          all = false;
          break;
        }
      }
    }

    if (all) {
      actions[interpolation]->setChecked(true);
    } else {
      actions[interpolation]->setChecked(false);
    }
  }
  return menu;
}

}  // namespace omm
