#include "managers/curvemanager/curvemanagerwidget.h"
#include "managers/panzoomcontroller.h"
#include "managers/curvemanager/curvetree.h"
#include "properties/property.h"
#include "scene/history/historymodel.h"
#include "scene/scene.h"
#include "commands/keyframecommand.h"
#include "mainwindow/application.h"
#include <QMouseEvent>
#include "scene/messagebox.h"
#include <QPainter>
#include <QEvent>
#include "managers/manager.h"
#include "scene/scene.h"
#include "animation/animator.h"

namespace omm
{

CurveManagerWidget::CurveManagerWidget(Scene& scene, const CurveTree& curve_tree)
  : range({1, -10}, {100, 10}, *this, Range::Options::Default, Range::Options::Mirror)
  , m_scene(scene), m_curve_tree(curve_tree)
{
  connect(&scene.message_box(), SIGNAL(selection_changed(const std::set<AbstractPropertyOwner*>&)),
          this, SLOT(set_selection(const std::set<AbstractPropertyOwner*>)));
  set_selection(scene.selection());
  connect(&scene.animator(), SIGNAL(track_inserted(Track&)), this, SLOT(add_track(Track&)));
  connect(&scene.animator(), SIGNAL(track_removed(Track&)), this, SLOT(remove_track(Track&)));
  connect(&scene.animator(), SIGNAL(knot_inserted(Track&, int)), this, SLOT(add_knot(Track&, int)));
  connect(&scene.animator(), SIGNAL(knot_removed(Track&, int)),
          this, SLOT(remove_knot(Track&, int)));
  connect(&scene.animator(), SIGNAL(knot_moved(Track&, int, int)),
          this, SLOT(move_knot(Track&, int, int)));
  setFocusPolicy(Qt::StrongFocus);
  connect(&curve_tree, SIGNAL(visibility_changed()), this, SLOT(update()));
  connect(&scene.animator(), SIGNAL(current_changed(int)), this, SLOT(update()));
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
    PanZoomController::draw_rubberband(painter, *this, rect.normalized());
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
    range.zoom(m_mouse_down_pos, d, {0.01, 0.00001}, {100.00, 10000.0});
  } else if (m_dragged_tangent.key != nullptr) {
    double old_y = value_range.pixel_to_unit(m_last_mouse_pos.y());
    double new_y = value_range.pixel_to_unit(event->y());
    const double diff = new_y - old_y;
    const KeyFrameHandleKey& key = *m_dragged_tangent.key;
    std::unique_ptr<Track::Knot> new_knot = key.track.knot(key.frame).clone();
    variant_type& vv = new_knot->offset(m_dragged_tangent.side);
    const double new_offset = get_channel_value(vv, key.channel) + diff;
    set_channel_value(vv, key.channel, new_offset);
    m_scene.submit<ChangeKeyFrameCommand>(key.frame, key.track.property(), std::move(new_knot));
  } else if (m_key_being_dragged) {
    m_frame_shift = std::round(  frame_range.pixel_to_unit(event->x())
                               - frame_range.pixel_to_unit(m_mouse_down_pos.x()));
    m_value_shift = value_range.pixel_to_unit(event->y())
                  - value_range.pixel_to_unit(m_mouse_down_pos.y());
  } else if (m_rubberband_rect_visible) {
    const QRectF rect = QRectF(range.pixel_to_unit(m_mouse_down_pos),
                               range.pixel_to_unit(m_last_mouse_pos)).normalized();
    for (auto&& [key, data] : m_keyframe_handles) {
      const QPointF p(key.frame, data.value(key));
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
      for (auto& k : ks) {
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
  static const auto is_selected_and_visible = [this](auto&& kd) {
    return kd.second.is_selected && is_visible(kd.first);
  };

  m_pan_active = false;
  m_zoom_active = false;
  if (m_key_being_dragged && (m_frame_shift != 0 || m_value_shift != 0)) {
    if (std::any_of(m_keyframe_handles.begin(), m_keyframe_handles.end(), is_selected_and_visible)) {
      auto& animator = m_scene.animator();
      auto macro = m_scene.history().start_macro("Move Keyframes");
      const auto keyframe_handles = m_keyframe_handles;
      for (auto&& [key, data] : keyframe_handles) {
        if (is_selected_and_visible(std::pair{ key, data })) {
          auto& property = key.track.property();
          const double new_value = data.value(key) + m_value_shift;
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
  return m_curve_tree.is_visible({ &track.property(), channel }) == CurveTree::Visibility::Visible;
}

const CurveManagerWidget::KeyFrameHandleKey*
CurveManagerWidget::neighbor(const CurveManagerWidget::KeyFrameHandleKey& key, Track::Knot::Side side) const
{
  const auto cmp = [side](int a, int b) {
    if (side == Track::Knot::Side::Left) {
      return a > b;
    } else {
      return a < b;
    }
  };
  const KeyFrameHandleKey* candidate_key = nullptr;
  for (const auto& pair : m_keyframe_handles) {
    if (&key.track == &pair.first.track) {
      if (  (candidate_key == nullptr || (cmp(pair.first.frame, candidate_key->frame)))
         && (cmp(key.frame, pair.first.frame)) )
      {
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
    const QPointF pos = range.unit_to_pixel(QPointF(key.frame, data.value(key)));
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
  const int frames_per_pixel = static_cast<double>(frame_range.end - frame_range.begin) / width();
  const int frame_advance = std::max(1, frames_per_pixel);
  for (Track* track_ : m_tracks) {
    for (std::size_t c = 0; c < n_channels(track_->property().variant_value()); ++c) {
      auto track = track_->clone();
      std::set<int> old_frames;
      for (auto&& [key, data] : m_keyframe_handles) {
        if (data.is_selected && &key.track == track_) {
          if (key.channel == c) {
            auto& v = track->knot(key.frame).value;
            const double sv = get_channel_value(v, c) + m_value_shift;
            set_channel_value(v, c, sv);
          }
          old_frames.insert(key.frame);
        }
      }
      for (auto it = old_frames.rbegin(); it != old_frames.rend(); ++it) {
        track->move_knot(*it, *it + m_frame_shift);
      }
      if (is_visible(*track, c)) {
        QPen pen;
        pen.setWidthF(1.5);
        const QString color_name = QString("%1-%2-fcurve").arg(track->type()).arg(c);
        pen.setColor(ui_color(QPalette::Active, "TimeLine", color_name));
        painter.setPen(pen);
        auto v0 = track->interpolate(std::floor(frame_range.begin - frame_advance), c);
        for (int frame = frame_range.begin;
             frame <= frame_range.end + frame_advance;
             frame += frame_advance)
        {
          auto v1 = track->interpolate(frame, c);
          painter.drawLine(range.unit_to_pixel(QPointF(frame - frame_advance, v0)),
                           range.unit_to_pixel(QPointF(frame, v1)));
          v0 = v1;
        }

        {
          const auto& property = track->property();
          const QString text = QString("%1 %2").arg(property.label()).arg(property.channel_name(c));
          const double b = frame_range.pixel_to_unit(0.0);
          const double e = frame_range.pixel_to_unit(painter.fontMetrics().horizontalAdvance(text));
          const double v1 = track->interpolate(b, c);
          const double v2 = track->interpolate(e, c);
          int y = range.v_range.unit_to_pixel(v1);
          if (v1 > v2) {
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
  const int left = range.h_range.unit_to_pixel(m_scene.animator().start() - 0.5);
  painter.fillRect(0, 0, left, height(), color);
  const int right = range.h_range.unit_to_pixel(m_scene.animator().end() + 0.5);
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

  static const std::map<double, Config> layers {
    { 100.0, { Qt::SolidLine, QColor(255, 255, 255, 80), 1.0, true } },
    { 10.0,  { Qt::DashLine, QColor(255, 255, 255, 20), 1.0, false } },
  };

  painter.save();
  for (auto [distance, config] : layers) {
    painter.setPen(make_pen(config));
    for (double y : range.v_range.scale(distance)) {
      const double py = range.v_range.unit_to_pixel(y);
      painter.drawLine(QPointF(0, py), QPointF(w, py));
      if (config.draw_text) {
        painter.drawText(QPointF(0, py-2), QString("%1").arg(y));
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
    const double any_channel_selected = std::any_of(m_keyframe_handles.begin(), m_keyframe_handles.end(),
                                                    [key=key](auto&& kd)
    {
      return &kd.first.track == &key.track && kd.first.frame == key.frame && kd.second.is_selected;
    });
    const double frame_shift = any_channel_selected ? m_frame_shift : 0.0;
    const double value_shift = data.is_selected ? m_value_shift : 0.0;
    if (is_visible(key.track, key.channel)) {
      const double value = data.value(key);
      const QPointF center = range.unit_to_pixel({key.frame + frame_shift, value + value_shift});
      painter.save();
      if (data.is_selected || data.inside_rubberband) {
        painter.setBrush(ui_color(QPalette::Active, "TimeLine", "key selected"));
        painter.drawEllipse(center, radius, radius);
        if (m_key_being_dragged) {
          const QPointF center = range.unit_to_pixel(QPointF(key.frame + m_frame_shift,
                                                             value + m_value_shift));
          painter.setBrush(ui_color(QPalette::Active, "TimeLine", "key dragged"));
          painter.drawEllipse(center, radius, radius);
        }
      } else {
        painter.setBrush(ui_color(QPalette::Active, "TimeLine", "key normal"));
        painter.drawEllipse(center, radius, radius);
      }

      if (key.track.interpolation() == Track::Interpolation::Bezier) {
        const auto draw_tangent = [this, center, &painter, key=key]
                                  (double value, double frame)
        {
          const QPointF other = range.unit_to_pixel({ frame, value });
          constexpr double r = 3;

          const QString color_name = QString("%1-%2-fcurve").arg(key.track.type()).arg(key.channel);
          QPen pen;
          const QColor color = ui_color(QPalette::Active, "TimeLine", color_name);
          pen.setColor(color);
          painter.setPen(pen);
          painter.setBrush(color);
          pen.setWidthF(1.2);
          painter.drawLine(center, other);
          painter.drawEllipse(other, r, r);
        };

        if (data.is_selected) {
          for (auto side : { Track::Knot::Side::Left, Track::Knot::Side::Right }) {
            const KeyFrameHandleKey* neighbor = this->neighbor(key, side);
            if (neighbor != nullptr) {
              const double value = key.value(side) + value_shift;
              const double frame = interpolate_frame(key.frame + frame_shift, neighbor->frame);
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
  return 2.0/3.0 * key_frame + 1.0/3.0 * neighbor_frame;
}

CurveManagerWidget::TangentHandle CurveManagerWidget::tangent_handle_at(const QPointF& point) const
{
  for (auto&& [key, data]: m_keyframe_handles) {
    for (auto side : { Track::Knot::Side::Left, Track::Knot::Side::Right }) {
      const auto* neighbor = this->neighbor(key, side);
      if (neighbor != nullptr) {
        const QPointF key_pos = range.unit_to_pixel({ interpolate_frame(key.frame, neighbor->frame),
                                                      key.value(side) });
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
      if (!::contains(new_tracks, track)) {
        remove_track(*track);
      }
    }

    for (Track* track : new_tracks) {
      if (!::contains(m_tracks, track)) {
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
  const auto [_, was_inserted] = m_tracks.insert(&track);
  Q_UNUSED(was_inserted)
  assert(was_inserted);
  update();
}

void CurveManagerWidget::remove_track(Track& track)
{
  for (int frame : track.key_frames()) {
    remove_knot(track, frame);
  }
  const std::size_t n = m_tracks.erase(&track);
  Q_UNUSED(n)
  assert(n == 1);
  update();
}

void CurveManagerWidget::add_knot(Track& track, int frame)
{
  for (std::size_t channel = 0; channel < n_channels(track.property().variant_value()); ++channel) {
    KeyFrameHandleKey key(track, frame, channel);
    m_keyframe_handles.insert({ key, KeyFrameHandleData() });
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
      m_keyframe_handles.insert({ key, old_data });
    }
  }
  update();
}

double CurveManagerWidget::KeyFrameHandleData
::value(const CurveManagerWidget::KeyFrameHandleKey& key) const
{
  return get_channel_value(key.track.knot(key.frame).value, key.channel);
}

double CurveManagerWidget::KeyFrameHandleKey::value(Track::Knot::Side side) const
{
  return get_channel_value(track.knot(frame).offset(side), channel) + value();
}

double CurveManagerWidget::KeyFrameHandleKey::value() const
{
  return get_channel_value(track.knot(frame).value, channel);
}

bool CurveManagerWidget::KeyFrameHandleKey::
operator<(const CurveManagerWidget::KeyFrameHandleKey& other) const
{
  if (&track != &other.track) {
    return &track < &other.track;
  } else if (frame != other.frame) {
    return frame < other.frame;
  } else {
    return channel < other.channel;
  }
}

variant_type& CurveManagerWidget::TangentHandle::offset()
{
  return key->track.knot(key->frame).offset(side);
}

}  // namespace omm
