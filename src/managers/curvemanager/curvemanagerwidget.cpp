#include "managers/curvemanager/curvemanagerwidget.h"
#include "animation/animator.h"
#include "commands/keyframecommand.h"
#include "main/application.h"
#include "managers/curvemanager/curvetreeview.h"
#include "managers/manager.h"
#include "managers/panzoomcontroller.h"
#include "properties/numericproperty.h"
#include "properties/property.h"
#include "preferences/uicolors.h"
#include "scene/history/historymodel.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include "scene/history/macro.h"
#include "preferences/preferences.h"
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>

namespace
{
double multiplier(const omm::Track& track)
{
  return track.property().configuration.get<double>(omm::NumericPropertyDetail::MULTIPLIER_POINTER);
}

void draw_rubberband(QPainter& painter, const QWidget& widget, const QRectF& rect)
{
  static constexpr double RUBBERBAND_WIDTH = 2.0;
  painter.save();
  painter.resetTransform();
  QPen pen;
  pen.setWidth(RUBBERBAND_WIDTH);
  pen.setCosmetic(true);
  pen.setColor(omm::ui_color(widget, "TimeLine", "rubberband outline"));
  painter.setPen(pen);
  painter.fillRect(rect, omm::ui_color(widget, "TimeLine", "rubberband fill"));
  painter.drawRect(rect);
  painter.restore();
}

constexpr int DEFAULT_MIN_H = 1;
constexpr int DEFAULT_MAX_H = 100;
constexpr int DEFAULT_MIN_V = -10;
constexpr int DEFAULT_MAX_V = 10;

}  // namespace

namespace omm
{
CurveManagerWidget::CurveManagerWidget(Scene& scene, const CurveTreeView& curve_tree)
    : range({DEFAULT_MIN_H, DEFAULT_MIN_V},
            {DEFAULT_MAX_H, DEFAULT_MAX_V},
            *this,
            Range::Options::Default,
            Range::Options::Mirror),
      m_scene(scene), m_curve_tree(curve_tree)
{
  connect(&scene.mail_box(), &MailBox::selection_changed, this, &CurveManagerWidget::set_selection);
  set_selection(scene.selection());
  connect(&scene.animator(), &Animator::track_inserted, this, &CurveManagerWidget::add_track);
  connect(&scene.animator(), &Animator::track_removed, this, &CurveManagerWidget::remove_track);
  connect(&scene.animator(), &Animator::knot_inserted, this, &CurveManagerWidget::add_knot);
  connect(&scene.animator(), &Animator::knot_removed, this, &CurveManagerWidget::remove_knot);
  connect(&scene.animator(), &Animator::knot_moved, this, &CurveManagerWidget::move_knot);
  setFocusPolicy(Qt::StrongFocus);
  connect(&curve_tree,
          &CurveTreeView::visibility_changed,
          this,
          qOverload<>(&CurveManagerWidget::update));
  connect(&scene.animator(),
          &Animator::current_changed,
          this,
          qOverload<>(&CurveManagerWidget::update));
}

void CurveManagerWidget::set_selection_locked(bool locked)
{
  m_selection_locked = locked;
}

void CurveManagerWidget::paintEvent(QPaintEvent* event)
{
  QPainter painter(this);
  painter.fillRect(rect(), ui_color(*this, "Widget", "base"));
  painter.setRenderHint(QPainter::TextAntialiasing);
  painter.setRenderHint(QPainter::Antialiasing);
  draw_background(painter);
  draw_scale(painter);
  draw_interpolation(painter);
  draw_knots(painter);
  if (m_rubberband_rect_visible) {
    const QRect rect(m_mouse_down_pos, m_last_mouse_pos);
    draw_rubberband(painter, *this, rect.normalized());
  }

  {
    painter.save();
    QPen pen;
    pen.setColor(ui_color(*this, "TimeLine", "slider outline"));
    painter.setPen(pen);
    const double x = range.h_range.unit_to_pixel(m_scene.animator().current());
    painter.drawLine(QPointF(x, 0), QPointF(x, height()));
    painter.restore();
  }

  QWidget::paintEvent(event);
}

void CurveManagerWidget::mouseMoveEvent(QMouseEvent* event)
{
  const Range& value_range = range.v_range;
  const Range& frame_range = range.h_range;
  const QPointF d = m_last_mouse_pos - event->pos();
  if (m_pan_active) {
    range.pan(QPointF(d.x() / width(), d.y() / height()));
  } else if (m_zoom_active) {
    static constexpr double LOWER_H = 0.01;
    static constexpr double LOWER_V = 0.00001;
    static constexpr double UPPER_H = 100.0;
    static constexpr double UPPER_V = 10000.0;
    range.zoom(m_mouse_down_pos, d, {LOWER_H, LOWER_V}, {UPPER_H, UPPER_V});
  } else if (m_dragged_tangent.key != nullptr) {
    double old_y = value_range.pixel_to_unit(m_last_mouse_pos.y());
    double new_y = value_range.pixel_to_unit(event->y());
    const double diff = new_y - old_y;
    const KeyFrameHandleKey& key = *m_dragged_tangent.key;
    std::unique_ptr<Knot> new_knot = key.track.knot(key.frame).clone();
    variant_type& vv = new_knot->offset(m_dragged_tangent.side);
    const double new_offset = get_channel_value(vv, key.channel) + diff;
    set_channel_value(vv, key.channel, new_offset);
    m_scene.submit<ChangeKeyFrameCommand>(key.frame, key.track.property(), std::move(new_knot));
  } else if (m_key_being_dragged) {
    m_frame_shift = static_cast<int>(std::round(frame_range.pixel_to_unit(event->x())
                                                - frame_range.pixel_to_unit(m_mouse_down_pos.x())));
    m_value_shift
        = value_range.pixel_to_unit(event->y()) - value_range.pixel_to_unit(m_mouse_down_pos.y());
  } else if (m_rubberband_rect_visible) {
    const QRectF rect
        = QRectF(range.pixel_to_unit(m_mouse_down_pos), range.pixel_to_unit(m_last_mouse_pos))
              .normalized();
    for (auto&& [key, data] : m_keyframe_handles) {
      const QPointF p(key.frame, key.value());
      data.inside_rubberband = rect.contains(p);
    }
  }
  m_last_mouse_pos = event->pos();
  update();
}

void CurveManagerWidget::mousePressEvent(QMouseEvent* event)
{
  m_mouse_down_pos = event->pos();
  m_last_mouse_pos = event->pos();
  m_pan_active = false;
  m_zoom_active = false;
  m_frame_shift = 0;
  m_value_shift = 0.0;
  m_action_aborted = false;
  m_rubberband_rect_visible = false;
  if (preferences().match("shift viewport", *event, true)) {
    m_pan_active = true;
  } else if (preferences().match("zoom viewport", *event, true)) {
    m_zoom_active = true;
  } else if (const auto ks = tangent_handle_at(event->pos()); ks.key != nullptr) {
    m_dragged_tangent = ks;
  } else if (const auto ks = keyframe_handles_at(event->pos()); !ks.empty()) {
    const bool is_selected = std::any_of(ks.begin(), ks.end(), [this](const auto* key) {
      return m_keyframe_handles.at(*key).is_selected && is_visible(*key);
    });
    if (is_selected) {
      // the clicked keyframe is selected
      m_key_being_dragged = true;
    } else {
      if (!(event->modifiers() & Qt::ShiftModifier)) {
        for (auto&& [key, data] : m_keyframe_handles) {
          data.is_selected = false;
        }
      }
      for (const auto& k : ks) {
        if (is_visible(*k)) {
          m_keyframe_handles.at(*k).is_selected = true;
          break;  // select only the first one.
        }
      }
      m_key_being_dragged = true;
    }
  } else {
    m_rubberband_rect_visible = true;
    if (!(event->modifiers() & Qt::ShiftModifier)) {
      for (auto&& [key, data] : m_keyframe_handles) {
        data.is_selected = false;
      }
    }
  }
  update();
}

void CurveManagerWidget::mouseReleaseEvent(QMouseEvent* event)
{
  static const auto is_selected_and_visible
      = [this](auto&& kd) { return kd.second.is_selected && is_visible(kd.first); };

  m_pan_active = false;
  m_zoom_active = false;
  if (m_key_being_dragged && (m_frame_shift != 0 || m_value_shift != 0)) {
    if (std::any_of(m_keyframe_handles.begin(),
                    m_keyframe_handles.end(),
                    is_selected_and_visible)) {
      auto& animator = m_scene.animator();
      auto macro = m_scene.history().start_macro("Move Keyframes");
      const auto keyframe_handles = m_keyframe_handles;
      for (auto&& [key, data] : keyframe_handles) {
        if (is_selected_and_visible(std::pair{key, data})) {
          auto& property = key.track.property();
          const double new_value = (key.value() + m_value_shift) / multiplier(key.track);
          auto new_knot = key.track.knot(key.frame).clone();
          set_channel_value(new_knot->value, key.channel, new_value);
          m_scene.submit<ChangeKeyFrameCommand>(key.frame, property, std::move(new_knot));
        }
      }

      std::map<Track*, std::set<int>> shifted_keyframes;
      for (auto&& [key, data] : keyframe_handles) {
        if (data.is_selected && is_visible(key)) {
          shifted_keyframes[&key.track].insert(key.frame);
        }
      }
      for (auto&& [track, frames] : shifted_keyframes) {
        m_scene.submit<MoveKeyFrameCommand>(animator, track->property(), frames, m_frame_shift);
      }
    }
    m_scene.animator().apply();
  } else if (!m_action_aborted) {
    for (const KeyFrameHandleKey* kfh : keyframe_handles_at(event->pos())) {
      m_keyframe_handles.at(*kfh).is_selected = true;
    }
  }

  for (auto&& [key, data] : m_keyframe_handles) {
    if (is_visible(key)) {
      data.is_selected |= data.inside_rubberband;
    }
    data.inside_rubberband = false;
  }

  m_rubberband_rect_visible = false;
  m_dragged_tangent = TangentHandle();
  m_key_being_dragged = false;
  m_frame_shift = 0;
  m_value_shift = 0.0;
  update();
  QWidget::mouseReleaseEvent(event);
}

void CurveManagerWidget::keyPressEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key_Escape) {
    m_key_being_dragged = false;
    m_value_shift = 0.0;
    m_frame_shift = 0;
    m_action_aborted = true;
    if (m_rubberband_rect_visible) {
      m_rubberband_rect_visible = false;
      for (auto&& [key, data] : m_keyframe_handles) {
        data.inside_rubberband = false;
      }
    }
  }
  update();
  QWidget::keyPressEvent(event);
}

bool CurveManagerWidget::is_visible(const CurveManagerWidget::KeyFrameHandleKey& key) const
{
  return is_visible(key.track, key.channel);
}

bool CurveManagerWidget::is_visible(const Track& track, std::size_t channel) const
{
  return m_curve_tree.is_visible({&track.property(), channel}) == CurveTreeView::Visibility::Visible;
}

const CurveManagerWidget::KeyFrameHandleKey*
CurveManagerWidget::neighbor(const CurveManagerWidget::KeyFrameHandleKey& key,
                             Knot::Side side) const
{
  const auto cmp = [side](int a, int b) {
    if (side == Knot::Side::Left) {
      return a > b;
    } else {
      return a < b;
    }
  };
  const KeyFrameHandleKey* candidate_key = nullptr;
  for (const auto& pair : m_keyframe_handles) {
    if (&key.track == &pair.first.track) {
      if ((candidate_key == nullptr || (cmp(pair.first.frame, candidate_key->frame)))
          && (cmp(key.frame, pair.first.frame))) {
        candidate_key = &pair.first;
      }
    }
  }
  return candidate_key;
}

std::set<const CurveManagerWidget::KeyFrameHandleKey*>
CurveManagerWidget::keyframe_handles_at(const QPointF& point) const
{
  std::set<const KeyFrameHandleKey*> its;

  for (auto&& [key, data] : m_keyframe_handles) {
    const QPointF pos = range.unit_to_pixel(QPointF(key.frame, key.value()));
    if (QPointF::dotProduct(pos - point, pos - point) < radius * radius) {
      its.insert(&key);
    }
  }
  return its;
}

void CurveManagerWidget::draw_interpolation(QPainter& painter) const
{
  painter.save();
  const Range& frame_range = range.h_range;
  // drawing the lines with a resolution of more than one sample per frame makes no sense.
  const double frames_per_pixel
      = static_cast<double>(frame_range.end - frame_range.begin) / width();
  const int frame_advance = static_cast<int>(std::max(1.0, frames_per_pixel));
  for (Track* track_ : m_tracks) {
    for (std::size_t c = 0; c < n_channels(track_->property().variant_value()); ++c) {
      auto track = track_->clone();
      std::set<int> old_frames;
      for (auto&& [key, data] : m_keyframe_handles) {
        if (data.is_selected && &key.track == track_) {
          if (key.channel == c) {
            auto& v = track->knot(key.frame).value;
            const double sv = get_channel_value(v, c) + m_value_shift / multiplier(key.track);
            set_channel_value(v, c, sv);
          }
          old_frames.insert(key.frame);
        }
      }
      for (auto it = old_frames.rbegin(); it != old_frames.rend(); ++it) {
        track->move_knot(*it, *it + m_frame_shift);
      }
      if (is_visible(*track, c)) {
        const double m = multiplier(*track);
        QPen pen;
        static constexpr double DEFAULT_PEN_WIDTH = 1.5;
        pen.setWidthF(DEFAULT_PEN_WIDTH);
        const QString color_name = QString("%1-%2-fcurve").arg(track->type()).arg(c);
        pen.setColor(ui_color(QPalette::Active, "TimeLine", color_name));
        painter.setPen(pen);

        {
          const int frame = std::floor(frame_range.begin - frame_advance);
          auto v0 = m * track->interpolate(frame, c);
          for (int frame = static_cast<int>(frame_range.begin);
               frame <= static_cast<int>(frame_range.end + frame_advance);
               frame += static_cast<int>(frame_advance)) {
            auto v1 = m * track->interpolate(frame, c);
            painter.drawLine(range.unit_to_pixel(QPointF(frame - frame_advance, v0)),
                             range.unit_to_pixel(QPointF(frame, v1)));
            v0 = v1;
          }
        }

        {
          const auto& property = track->property();
          const QString text = QString("%1 %2").arg(property.label(), property.channel_name(c));
          const double b = frame_range.pixel_to_unit(0.0);
          const double e = frame_range.pixel_to_unit(painter.fontMetrics().horizontalAdvance(text));
          const double v1 = m * track->interpolate(b, c);
          const double v2 = m * track->interpolate(e, c);
          int y = static_cast<int>(range.v_range.unit_to_pixel(v1));
          if (v1 < v2) {
            y += painter.fontMetrics().height();
          } else {
            y -= 2;
          }
          painter.drawText(QPointF(0, y), text);
        }
      }
    }
  }
  painter.restore();
}

void CurveManagerWidget::draw_background(QPainter& painter) const
{
  const QColor color = ui_color(*this, "TimeLine", "beyond");
  const int left = static_cast<int>(range.h_range.unit_to_pixel(m_scene.animator().start() - 0.5));
  painter.fillRect(0, 0, left, height(), color);
  const int right = static_cast<int>(range.h_range.unit_to_pixel(m_scene.animator().end() + 0.5));
  painter.fillRect(std::max(0, right), 0, width(), height(), color);
}

void CurveManagerWidget::draw_scale(QPainter& painter) const
{
  const int w = width();
  const int h = height();

  struct Config {
    Qt::PenStyle style;
    QColor color;
    double width;
    bool draw_text;
  };

  static const auto make_pen = [](const Config& config) {
    QPen pen;
    pen.setStyle(config.style);
    pen.setColor(config.color);
    pen.setWidthF(config.width);
    return pen;
  };

  static const std::map<double, Config> layers{
      {100.0, {Qt::SolidLine, QColor(255, 255, 255, 80), 1.0, true}},
      {10.0, {Qt::DashLine, QColor(255, 255, 255, 20), 1.0, false}},
  };

  painter.save();
  for (const auto& [distance, config] : layers) {
    painter.setPen(make_pen(config));
    for (double y : range.v_range.scale(distance)) {
      const double py = range.v_range.unit_to_pixel(y);
      painter.drawLine(QPointF(0, py), QPointF(w, py));
      if (config.draw_text) {
        painter.drawText(QPointF(0, py - 2), QString("%1").arg(y));
      }
    }
    for (double x : range.h_range.scale(distance)) {
      const double px = range.h_range.unit_to_pixel(x);
      painter.drawLine(QPointF(px, 0), QPointF(px, h));
      if (config.draw_text) {
        painter.drawText(QPointF(px, painter.fontMetrics().height()), QString("%1").arg(x));
      }
    }
  }
  painter.restore();
}

void CurveManagerWidget::draw_knots(QPainter& painter) const
{
  for (const auto& [key, data] : m_keyframe_handles) {
    const bool any_channel_selected
        = std::any_of(m_keyframe_handles.begin(), m_keyframe_handles.end(), [key = key](auto&& kd) {
            return &kd.first.track == &key.track && kd.first.frame == key.frame
                   && kd.second.is_selected;
          });
    const double frame_shift = static_cast<int>(any_channel_selected ? m_frame_shift : 0.0);
    const double value_shift = data.is_selected ? m_value_shift : 0.0;
    if (is_visible(key.track, key.channel)) {
      const double value = key.value();
      const QPointF center = range.unit_to_pixel({key.frame + frame_shift, value + value_shift});
      painter.save();
      if (data.is_selected || data.inside_rubberband) {
        painter.setBrush(ui_color(QPalette::Active, "TimeLine", "key selected"));
        painter.drawEllipse(center, radius, radius);
        if (m_key_being_dragged) {
          const QPointF center
              = range.unit_to_pixel(QPointF(key.frame + m_frame_shift, value + m_value_shift));
          painter.setBrush(ui_color(QPalette::Active, "TimeLine", "key dragged"));
          painter.drawEllipse(center, radius, radius);
        }
      } else {
        painter.setBrush(ui_color(QPalette::Active, "TimeLine", "key normal"));
        painter.drawEllipse(center, radius, radius);
      }

      if (key.track.interpolation() == Track::Interpolation::Bezier) {
        const auto draw_tangent = [this, center, &painter, key = key](double value, double frame) {
          const QPointF other = range.unit_to_pixel({frame, value});
          constexpr double r = 3;

          const QString color_name = QString("%1-%2-fcurve").arg(key.track.type()).arg(key.channel);
          QPen pen;
          const QColor color = ui_color(QPalette::Active, "TimeLine", color_name);
          pen.setColor(color);
          painter.setPen(pen);
          painter.setBrush(color);
          static constexpr double DEFAULT_PEN_WIDTH = 1.2;
          pen.setWidthF(DEFAULT_PEN_WIDTH);
          painter.drawLine(center, other);
          painter.drawEllipse(other, r, r);
        };

        if (data.is_selected) {
          for (auto side : {Knot::Side::Left, Knot::Side::Right}) {
            const KeyFrameHandleKey* neighbor = this->neighbor(key, side);
            if (neighbor != nullptr) {
              const double value = key.value(side) + value_shift;
              const double frame
                  = interpolate_frame(key.frame + static_cast<int>(frame_shift), neighbor->frame);
              draw_tangent(value, frame);
            }
          }
        }
      }

      painter.restore();
    }
  }
}

double CurveManagerWidget::interpolate_frame(int key_frame, int neighbor_frame)
{
  static constexpr double t = 2.0 / 3.0;
  return t * key_frame + (1.0 - t) * neighbor_frame;
}

CurveManagerWidget::TangentHandle CurveManagerWidget::tangent_handle_at(const QPointF& point) const
{
  for (auto&& [key, data] : m_keyframe_handles) {
    for (auto side : {Knot::Side::Left, Knot::Side::Right}) {
      const auto* neighbor = this->neighbor(key, side);
      if (neighbor != nullptr) {
        const QPointF key_pos
            = range.unit_to_pixel({interpolate_frame(key.frame, neighbor->frame), key.value(side)});
        if ((key_pos - point).manhattanLength() < 3) {
          return TangentHandle(&key, side);
        }
      }
    }
  }
  return TangentHandle();
}

void CurveManagerWidget::set_selection(const std::set<AbstractPropertyOwner*>& selection)
{
  if (!m_selection_locked) {
    std::set<Track*> new_tracks;
    for (AbstractPropertyOwner* apo : selection) {
      for (Property* property : apo->properties().values()) {
        if (Track* track = property->track(); track != nullptr) {
          new_tracks.insert(track);
        }
      }
    }

    const auto old_tracks = m_tracks;

    for (Track* track : old_tracks) {
      if (!new_tracks.contains(track)) {
        remove_track(*track);
      }
    }

    for (Track* track : new_tracks) {
      if (!m_tracks.contains(track)) {
        add_track(*track);
      }
    }

    assert(m_tracks == new_tracks);
  }
}

void CurveManagerWidget::add_track(Track& track)
{
  for (int frame : track.key_frames()) {
    add_knot(track, frame);
  }
  [[maybe_unused]] const auto [_, was_inserted] = m_tracks.insert(&track);
  assert(was_inserted);
  update();
}

void CurveManagerWidget::remove_track(Track& track)
{
  for (int frame : track.key_frames()) {
    remove_knot(track, frame);
  }
  [[maybe_unused]] const std::size_t n = m_tracks.erase(&track);
  assert(n == 1);
  update();
}

void CurveManagerWidget::add_knot(Track& track, int frame)
{
  for (std::size_t channel = 0; channel < n_channels(track.property().variant_value()); ++channel) {
    KeyFrameHandleKey key(track, frame, channel);
    m_keyframe_handles.insert({key, KeyFrameHandleData()});
  }
  update();
}

void CurveManagerWidget::remove_knot(Track& track, int frame)
{
  for (std::size_t channel = 0; channel < n_channels(track.property().variant_value()); ++channel) {
    const auto it = m_keyframe_handles.find(KeyFrameHandleKey(track, frame, channel));
    if (it != m_keyframe_handles.end()) {
      m_keyframe_handles.erase(it);
    }
  }
  update();
}

void CurveManagerWidget::move_knot(Track& track, int old_frame, int new_frame)
{
  for (std::size_t channel = 0; channel < n_channels(track.property().variant_value()); ++channel) {
    const auto it = m_keyframe_handles.find(KeyFrameHandleKey(track, old_frame, channel));
    if (it != m_keyframe_handles.end()) {
      const auto node = m_keyframe_handles.extract(it);
      const auto old_key = node.key();
      const auto old_data = node.mapped();

      KeyFrameHandleKey key(old_key.track, new_frame, old_key.channel);
      m_keyframe_handles.insert({key, old_data});
    }
  }
  update();
}

double CurveManagerWidget::KeyFrameHandleKey::value(Knot::Side side) const
{
  const double v = get_channel_value(track.knot(frame).offset(side), channel);
  return multiplier(track) * v + value();
}

double CurveManagerWidget::KeyFrameHandleKey::value() const
{
  return multiplier(track) * get_channel_value(track.knot(frame).value, channel);
}

bool CurveManagerWidget::KeyFrameHandleKey::operator<(
    const CurveManagerWidget::KeyFrameHandleKey& other) const
{
  if (&track != &other.track) {
    return &track < &other.track;
  } else if (frame != other.frame) {
    return frame < other.frame;
  } else {
    return channel < other.channel;
  }
}

variant_type& CurveManagerWidget::TangentHandle::offset() const
{
  return key->track.knot(key->frame).offset(side);
}

}  // namespace omm
