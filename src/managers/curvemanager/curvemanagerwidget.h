#pragma once

#include "animation/knot.h"
#include "logging.h"
#include "managers/range.h"
#include <QWidget>
#include <set>

namespace omm
{
class Manager;
class CurveTreeView;
class Track;
class Scene;

class CurveManagerWidget : public QWidget
{
  Q_OBJECT
public:
  explicit CurveManagerWidget(Scene& scene, const CurveTreeView& curve_tree);
  void set_selection_locked(bool locked);

protected:
  void paintEvent(QPaintEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;

private:
  WidgetRange2D range;
  Scene& m_scene;
  const CurveTreeView& m_curve_tree;
  bool m_selection_locked = false;
  std::set<Track*> m_tracks;
  QPoint m_mouse_down_pos;
  QPoint m_last_mouse_pos;
  bool m_pan_active = false;
  bool m_zoom_active = false;
  bool m_key_being_dragged = false;
  int m_frame_shift = 0;
  double m_value_shift = 0.0;
  bool m_action_aborted = false;
  bool m_rubberband_rect_visible = false;
  static constexpr double radius = 5.0;

  struct KeyFrameHandleKey {
    KeyFrameHandleKey(Track& track, int frame, std::size_t channel)
        : track(track), frame(frame), channel(channel)
    {
    }
    Track& track;
    const int frame;
    const std::size_t channel;
    [[nodiscard]] double value() const;
    [[nodiscard]] double value(Knot::Side side) const;
    bool operator<(const KeyFrameHandleKey& other) const;
  };

  struct KeyFrameHandleData {
    bool is_selected = false;
    bool inside_rubberband = false;
  };

  std::map<KeyFrameHandleKey, KeyFrameHandleData> m_keyframe_handles;
  [[nodiscard]] bool is_visible(const KeyFrameHandleKey& key) const;
  [[nodiscard]] bool is_visible(const Track& track, std::size_t channel) const;
  [[nodiscard]] const KeyFrameHandleKey* neighbor(const KeyFrameHandleKey& key,
                                                  Knot::Side side) const;

  [[nodiscard]] std::set<const KeyFrameHandleKey*> keyframe_handles_at(const QPointF& point) const;

  void draw_interpolation(QPainter& painter) const;
  void draw_background(QPainter& painter) const;
  void draw_scale(QPainter& painter) const;
  void draw_knots(QPainter& painter) const;

  struct TangentHandle {
    TangentHandle() = default;
    TangentHandle(const KeyFrameHandleKey* key, Knot::Side side) : key(key), side(side)
    {
    }

    const KeyFrameHandleKey* key{nullptr};
    Knot::Side side = Knot::Side::Left;
    [[nodiscard]] variant_type& offset() const;
  };

  static double interpolate_frame(int key, int neighbor);

  [[nodiscard]] TangentHandle tangent_handle_at(const QPointF& point) const;
  TangentHandle m_dragged_tangent = TangentHandle();

private:
  void set_selection(const std::set<AbstractPropertyOwner*>& selection);
  void add_track(omm::Track& track);
  void remove_track(omm::Track& track);
  void add_knot(omm::Track& track, int frame);
  void remove_knot(omm::Track& track, int frame);
  void move_knot(omm::Track& track, int old_frame, int new_frame);
};

}  // namespace omm
