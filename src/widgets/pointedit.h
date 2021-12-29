#pragma once

#include <QWidget>

class QPushButton;

namespace omm
{
class CoordinateEdit;
class PathObject;
class Point;
class PathPoint;
struct PolarCoordinates;
enum class DisplayMode;

class PointEdit : public QWidget
{
  Q_OBJECT
public:
  PointEdit(PathObject& path_object, PathPoint& point, QWidget* parent = nullptr);
  void set_display_mode(const DisplayMode& display_mode);

private:
  QPushButton* m_mirror_from_left = nullptr;
  QPushButton* m_mirror_from_right = nullptr;
  QPushButton* m_coupled;
  QPushButton* m_vanish_left = nullptr;
  QPushButton* m_vanish_right = nullptr;
  CoordinateEdit* m_left_tangent_edit = nullptr;
  CoordinateEdit* m_right_tangent_edit = nullptr;
  CoordinateEdit* m_position_edit;
  PathPoint& m_point;
  PathObject* m_path{};

private:
  void mirror_from_right();
  void mirror_from_left();
  void set_left_maybe(const omm::PolarCoordinates& old_right,
                      const omm::PolarCoordinates& new_right);
  void set_right_maybe(const omm::PolarCoordinates& old_left,
                       const omm::PolarCoordinates& new_left);
  void update_point();
};

}  // namespace omm
