#include "managers/timeline/timelinecanvas.h"
#include "animation/animator.h"
#include "animation/track.h"
#include "animation/knot.h"
#include "commands/keyframecommand.h"
#include "logging.h"
#include "main/application.h"
#include "preferences/uicolors.h"
#include "preferences/preferences.h"
#include "scene/history/historymodel.h"
#include "scene/history/macro.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include "removeif.h"
#include <QCursor>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWidget>
#include <cmath>

namespace
{
bool frame_has_line(double ppfs, int frame)
{
  static const std::list<std::pair<double, int>> lods{{10.0, 2}, {2.0, 10}, {1.0, 20}};
  return std::any_of(lods.begin(), lods.end(), [frame, ppfs](auto&& arg) {
    auto&& [ppfs_threshold, frame_div] = arg;
    return ppfs >= ppfs_threshold && (frame % frame_div == 0);
  });
}

bool frame_has_framenumber(double ppfs, int frame)
{
  static constexpr double inf = std::numeric_limits<double>::infinity();
  static const std::list<std::pair<double, int>> lods{{2.0, 100}, {10.0, 20}, {20.0, 10}, {inf, 2}};
  for (auto&& [ppfs_threshold, frame_div] : lods) {
    if (ppfs < ppfs_threshold) {
      return frame % frame_div == 0;
    }
  }
  return false;
}
}  // namespace

namespace omm
{
TimelineCanvas::TimelineCanvas(Animator& animator, QWidget& widget)
    : animator(animator), frame_range(*this), m_widget(widget)
{
  connect(&animator, &Animator::knot_removed, this, [this](Track& track, int frame) {
    if (const auto it = m_selection.find(&track); it != m_selection.end()) {
      it->second.erase(frame);
      if (it->second.empty()) {
        m_selection.erase(it);
      }
    }
  });
  connect(&animator,
          &Animator::knot_moved,
          this,
          [this](Track& track, int old_frame, int new_frame) {
            if (const auto& it = m_selection.find(&track); it != m_selection.end()) {
              if (it->second.erase(old_frame) > 0) {
                it->second.insert(new_frame);
              }
            }
          });
}

void TimelineCanvas::draw_background(QPainter& painter) const
{
  painter.save();
  painter.translate(rect.topLeft());
  painter.scale(rect.width(), rect.height());
  const double ppf = 1.0 / (frame_range.end - frame_range.begin);
  const double left = (animator.start() - frame_range.begin) * ppf - ppf / 2.0;
  const double right = (animator.end() - frame_range.begin) * ppf + ppf / 2.0;
  if (left > 0.0) {
    painter.fillRect(QRectF(QPointF(0, 0), QPointF(left, 1.0)),
                     ui_color(m_widget, "TimeLine", "beyond"));
  }
  if (right < 1.0) {
    painter.fillRect(QRectF(QPointF(right, 0), QPointF(1.0, 1.0)),
                     ui_color(m_widget, "TimeLine", "beyond"));
  }
  if (right > 0.0 && left < 1.0) {
    painter.fillRect(QRectF(QPointF(left, 0.0), QPointF(right, 1.0)),
                     ui_color(m_widget, QPalette::Base));
  }
  painter.restore();
}

void TimelineCanvas::draw_lines(QPainter& painter) const
{
  painter.save();

  QPen pen;
  pen.setColor(ui_color(m_widget, "TimeLine", "scale vline"));
  pen.setCosmetic(true);
  painter.setPen(pen);

  const double ppf = 1.0 / (frame_range.end - frame_range.begin);
  const double ppfs = ppf * frame_range.pixel_range();

  for (int frame = static_cast<int>(frame_range.begin);
       frame <= static_cast<int>(frame_range.end + 1.0);
       ++frame) {
    if (frame_has_line(ppfs, frame)) {
      static constexpr double THICK_LINE_WIDTH = 1.0;
      static constexpr double THIN_LINE_WIDTH = 0.5;
      static constexpr int THICK_LINE_SPACING = 10;
      pen.setWidthF(frame % THICK_LINE_SPACING == 0 ? THICK_LINE_WIDTH : THIN_LINE_WIDTH);
      painter.setPen(pen);

      const double x = (frame - frame_range.begin) * ppf;
      const double line_start = frame % 2 == 0 ? 0 : 0.05;

      // there is no way in drawing really tiny text. Hence, we must draw the frame numbers in
      // non-normalized coordinates...
      painter.save();
      painter.resetTransform();
      painter.translate(rect.left() + rect.width() * x, rect.top());

      const double line_end = footer_height > 0 ? std::max(line_start, footer_y()) : rect.height();
      if (line_end != line_start) {
        painter.drawLine(QPointF(0, line_start), QPointF(0, line_end));
      }

      if (footer_height > 0 && frame_has_framenumber(ppfs, frame)) {
        const QString text = QString("%1").arg(frame);
        const QFontMetricsF fm(painter.font());
        const double text_width = fm.horizontalAdvance(text);
        const double margin = (footer_height - fm.height()) / 2.0;
        painter.drawText(QPointF(-text_width / 2.0, footer_y() + margin + fm.height()), text);
      }
      painter.restore();
    }
  }

  painter.restore();
}

void TimelineCanvas::draw_keyframes(QPainter& painter) const
{
  painter.save();
  painter.translate(rect.topLeft());
  const auto y = static_cast<int>(footer_y() / 2.0);

  for (int frame = static_cast<int>(frame_range.begin);
       frame <= static_cast<int>(frame_range.end + 1.0);
       ++frame) {
    const bool draw = std::any_of(tracks.begin(), tracks.end(), [frame](const Track* track) {
      return track->has_keyframe(frame);
    });
    if (draw) {
      const bool is_selected = this->is_selected(frame);
      draw_keyframe(painter,
                    frame,
                    y,
                    is_selected ? KeyFrameStatus::Selected : KeyFrameStatus::Normal);
      if (m_shift != 0 && is_selected) {
        draw_keyframe(painter, frame + m_shift, y, KeyFrameStatus::Dragged);
      }
    }
  }
  painter.restore();
}

void TimelineCanvas::draw_current(QPainter& painter) const
{
  painter.save();
  painter.translate(rect.topLeft());
  const double x = frame_range.unit_to_pixel(animator.current());
  const double fpp = 1.0 / (frame_range.end - frame_range.begin);
  const QRectF current_rect(QPointF(x - fpp / 2.0, footer_y()), QSizeF(fpp, footer_height));
  painter.fillRect(current_rect, ui_color(m_widget, "TimeLine", "slider fill"));
  QPen pen;
  pen.setColor(ui_color(m_widget, "TimeLine", "slider outline"));
  static constexpr double CURRENT_FRAME_LINE_WIDTH = 4.0;
  pen.setWidthF(CURRENT_FRAME_LINE_WIDTH);
  pen.setCosmetic(true);
  painter.setPen(pen);
  painter.drawRect(current_rect);
  painter.restore();
}

void TimelineCanvas::draw_rubber_band(QPainter& painter) const
{
  if (m_rubber_band_visible) {
    painter.save();
    QPen pen;
    pen.setWidth(2.0);
    pen.setColor(ui_color(m_widget, "TimeLine", "rubberband outline"));
    painter.setPen(pen);
    painter.fillRect(rubber_band(), ui_color(m_widget, "TimeLine", "rubberband fill"));
    painter.drawRect(rubber_band());
    painter.restore();
  }
}

bool TimelineCanvas::view_event(QEvent& event)
{
  switch (event.type()) {
  case QEvent::MouseButtonPress:
    return mouse_press(dynamic_cast<QMouseEvent&>(event));
  case QEvent::MouseButtonRelease:
    return mouse_release(dynamic_cast<QMouseEvent&>(event));
  case QEvent::MouseMove:
    return mouse_move(dynamic_cast<QMouseEvent&>(event));
  case QEvent::KeyPress:
    return key_press(dynamic_cast<QKeyEvent&>(event));
  default:
    return false;
  }
}

bool TimelineCanvas::mouse_press(QMouseEvent& event)
{
  m_mouse_down_pos = event.pos();
  m_last_mouse_pos = event.pos();
  m_shift = 0;
  m_move_aborted = false;
  m_pan_active = false;
  m_zoom_active = false;
  const double dframe = std::round(frame_range.pixel_to_unit(event.pos().x() - rect.left()));
  const int frame = static_cast<int>(dframe);
  if (preferences().match("shift viewport", event, true)) {
    m_pan_active = true;
    disable_context_menu();
    return true;
  } else if (preferences().match("zoom viewport", event, true)) {
    disable_context_menu();
    m_zoom_active = true;
    return true;
  } else if (event.pos().y() - rect.top() < footer_y()) {
    if (event.button() == Qt::LeftButton) {
      if (!(event.modifiers() & Qt::ShiftModifier) && !is_selected(frame)) {
        m_selection.clear();
      }
      if (const auto tracks = tracks_at(frame); !tracks.empty()) {
        select(frame);
        update();
        m_dragging_knots = true;
      } else {
        m_rubber_band_visible = true;
        m_rubber_band_origin = event.pos();
        m_rubber_band_corner = m_rubber_band_origin;
      }
      return true;
    }
  } else {
    if (event.button() == Qt::LeftButton) {
      m_dragging_time = true;
      const int diff = static_cast<int>(event.pos().x() - rect.left());
      Q_EMIT current_frame_changed(static_cast<int>(std::round(frame_range.pixel_to_unit(diff))));
      return true;
    }
  }
  return false;
}

bool TimelineCanvas::mouse_move(QMouseEvent& event)
{
  QPointF d = QPointF(m_last_mouse_pos - event.pos());
  m_last_mouse_pos = event.pos();
  if (m_pan_active) {
    pan(QPointF(d.x() / rect.width(), d.y() / rect.height()));
    update();
  } else if (m_zoom_active) {
    zoom(d);
    update();
  } else if (m_dragging_knots && !m_move_aborted) {
    m_shift = static_cast<int>(std::round(frame_range.pixel_to_unit(event.x())
                                          - frame_range.pixel_to_unit(m_mouse_down_pos.x())));
    update();
  } else if (m_rubber_band_visible && !m_move_aborted) {
    const QPoint pos = event.pos();
    if (rect.isEmpty()) {
      m_rubber_band_corner = pos;
    } else {
      m_rubber_band_corner = QPoint(std::clamp<int>(pos.x(), rect.left(), rect.right()),
                                    std::clamp<int>(pos.y(), rect.top(), rect.bottom()));
    }

    const auto compute_x = [this, left = rect.left()](int pos) {
      return static_cast<int>(std::lround(frame_range.pixel_to_unit(pos) - left));
    };

    const int left = compute_x(rubber_band().left());
    const int right = compute_x(rubber_band().right());

    for (Property* property : animator.accelerator().properties()) {
      Track& track = *property->track();
      if (track_rect(track).intersects(rubber_band())) {
        m_rubber_band_selection[&track].clear();
        for (int frame : track.key_frames()) {
          if (left <= frame && frame <= right) {
            m_rubber_band_selection[&track].insert(frame);
          }
        }
      } else {
        m_rubber_band_selection.erase(&track);
      }
    }
    for (AbstractPropertyOwner* owner : animator.accelerator().owners()) {
      if (owner_rect(*owner).intersects(rubber_band())) {
        for (Property* property : animator.accelerator().properties(*owner)) {
          Track& track = *property->track();
          for (int frame : track.key_frames()) {
            if (left <= frame && frame <= right) {
              m_rubber_band_selection[&track].insert(frame);
            }
          }
        }
      }
    }

    update();
  } else if (m_dragging_time) {
    double x = frame_range.pixel_to_unit(event.pos().x() - rect.left());
    Q_EMIT current_frame_changed(static_cast<int>(std::round(x)));
  } else {
    return false;
  }
  return true;
}

bool TimelineCanvas::mouse_release(QMouseEvent& event)
{
  m_rubber_band_visible = false;
  for (auto&& [track, selection] : m_rubber_band_selection) {
    m_selection[track].insert(selection.begin(), selection.end());
  }
  m_rubber_band_selection.clear();
  const int frame
      = static_cast<int>(std::round(frame_range.pixel_to_unit(event.pos().x() - rect.left())));
  if (m_shift == 0 && !m_move_aborted && m_dragging_knots) {
    if (!(event.modifiers() & Qt::ShiftModifier)) {
      m_selection.clear();
    }
    select(frame);
  } else if (!m_move_aborted && m_shift != 0) {
    assert(m_dragging_knots);  // m_shift != 0 imples m_dragging_knots
    std::list<std::unique_ptr<MoveKeyFrameCommand>> commands;
    for (auto&& [track, selected_frames] : m_selection) {
      commands.push_back(std::make_unique<MoveKeyFrameCommand>(animator,
                                                               track->property(),
                                                               selected_frames,
                                                               m_shift));
    }
    if (!commands.empty()) {
      [[maybe_unused]] auto macro = animator.scene.history().start_macro(QObject::tr("Move knots"));
      for (auto&& command : commands) {
        animator.scene.submit(std::move(command));
      }
    }
  }
  Q_UNUSED(event)
  m_pan_active = false;
  m_zoom_active = false;
  m_dragging_knots = false;
  m_dragging_time = false;
  m_shift = 0;
  update();
  enable_context_menu();
  return false;
}

bool TimelineCanvas::key_press(QKeyEvent& event)
{
  if (event.key() == Qt::Key_Escape) {
    m_move_aborted = true;
    m_shift = 0;
    update();
    return true;
  } else {
    return false;
  }
}

QRect TimelineCanvas::rubber_band() const
{
  return QRect(m_rubber_band_corner, m_rubber_band_origin).normalized();
}

void TimelineCanvas::pan(const QPointF& d)
{
  frame_range.pan(d.x());
}

void TimelineCanvas::zoom(const QPointF& d)
{
  const double min_ppf = 0.02;
  const double max_ppf = 2.0;
  frame_range.zoom(m_mouse_down_pos.x() - rect.left(), d.x(), min_ppf, max_ppf);
}

void TimelineCanvas::synchronize_track_selection_with_animator()
{
  connect(&animator.scene.mail_box(),
          &MailBox::selection_changed,
          this,
          &TimelineCanvas::update_tracks);
  update_tracks(animator.scene.selection());
  connect(&animator, &Animator::track_inserted, this, [this](Track& track) {
    tracks.insert(&track);
    update();
  });
  connect(&animator, &Animator::track_removed, this, [this](Track& track) {
    tracks.erase(&track);
    update();
  });
  connect(&animator, &Animator::start_changed, this, &TimelineCanvas::update);
  connect(&animator, &Animator::end_changed, this, &TimelineCanvas::update);
  connect(&animator, &Animator::track_changed, this, &TimelineCanvas::update);
  connect(&animator.scene.mail_box(), &MailBox::selection_changed, this, &TimelineCanvas::update);
}

std::set<Track*> TimelineCanvas::tracks_at(double frame) const
{
  std::set<Track*> tracks;
  for (Track* track : this->tracks) {
    if (track->has_keyframe(static_cast<int>(std::round(frame)))) {
      tracks.insert(track);
    }
  }
  return tracks;
}

double TimelineCanvas::footer_y() const
{
  return rect.height() - footer_height;
}

bool TimelineCanvas::is_selected(int frame) const
{
  const auto relevant_tracks = util::remove_if(tracks, [frame](const Track* const track) {
    return !track->has_keyframe(frame);
  });
  if (relevant_tracks.empty()) {
    return false;
  }

  return std::all_of(relevant_tracks.begin(), relevant_tracks.end(), [this, frame](Track* const track) {
    const auto is_selected = [frame, track](auto&& selection) {
      const auto it = selection.find(track);
      return it != selection.end() && it->second.contains(frame);
    };
    return is_selected(m_selection) || is_selected(m_rubber_band_selection);
  });
}

void TimelineCanvas::select(int frame)
{
  for (Track* track : tracks) {
    if (track->has_keyframe(frame)) {
      m_selection[track].insert(frame);
    }
  }
}

void TimelineCanvas::update_tracks(const std::set<AbstractPropertyOwner*>& selection)
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

void TimelineCanvas ::draw_keyframe(QPainter& painter,
                                    int frame,
                                    double y,
                                    KeyFrameStatus status) const
{
  static const std::map<KeyFrameStatus, QString> status_name_map = {
      {KeyFrameStatus::Dragged, "key dragged"},
      {KeyFrameStatus::Normal, "key normal"},
      {KeyFrameStatus::Selected, "key selected"},
  };
  static const QPainterPath diamond = []() {
    QPainterPath diamond;
    diamond.moveTo(QPointF(0, 1));
    diamond.lineTo(QPointF(1, 0));
    diamond.lineTo(QPointF(0, -1));
    diamond.lineTo(QPointF(-1, 0));
    diamond.closeSubpath();
    return diamond;
  }();
  QPen pen;
  pen.setCosmetic(true);
  pen.setColor(ui_color(m_widget, "TimeLine", "key outline"));
  static constexpr double MIN_X_SCALE = 4.0;
  static constexpr double MAX_X_SCALE = 4.0;
  static constexpr double MIN_Y_SCALE = 20.0;
  const QPointF scale(std::clamp(rect.width() / (frame_range.end - frame_range.begin) / 2.0,
                                 MIN_X_SCALE,
                                 MAX_X_SCALE),
                      std::min(footer_y() / 2.0, MIN_Y_SCALE));
  static constexpr double PEN_WIDTH_SCALE = 0.2;
  pen.setWidthF(std::max(0.0, std::min(scale.x(), scale.y()) * PEN_WIDTH_SCALE));
  painter.setPen(pen);
  painter.save();
  painter.translate(frame_range.unit_to_pixel(frame), y);
  static constexpr double PAINTER_SCALE = 0.8;
  painter.scale(scale.x() * PAINTER_SCALE, scale.y() * PAINTER_SCALE);
  painter.fillPath(diamond, ui_color(m_widget, "TimeLine", status_name_map.at(status)));
  painter.drawPath(diamond);
  painter.restore();
}

TimelineCanvas::PixelRange::PixelRange(TimelineCanvas& self)
    : Range(Animator::DEFAULT_START_FRAME, Animator::DEFAULT_END_FRAME), m_self(self)
{
}

int TimelineCanvas::PixelRange::pixel_range() const
{
  return m_self.rect.width();
}

}  // namespace omm
