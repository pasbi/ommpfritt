#include "widgets/pointedit.h"
#include "commands/modifypointscommand.h"
#include "scene/scene.h"
#include "objects/pathobject.h"
#include "path/pathpoint.h"
#include "widgets/coordinateedit.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <memory>

namespace
{
auto make_tangent_layout(omm::CoordinateEdit*& coordinate_edit_ref,
                         QPushButton*& mirror_button_ref,
                         QPushButton*& vanish_button_ref)
{
  auto coordinate_edit = std::make_unique<omm::CoordinateEdit>();
  coordinate_edit_ref = coordinate_edit.get();
  auto vanish_button = std::make_unique<QPushButton>("V");
  vanish_button_ref = vanish_button.get();
  auto mirror_button = std::make_unique<QPushButton>(">|<");
  mirror_button_ref = mirror_button.get();

  auto hlayout = std::make_unique<QHBoxLayout>();
  hlayout->addWidget(vanish_button.release());
  hlayout->addWidget(mirror_button.release());

  auto vlayout = std::make_unique<QVBoxLayout>();
  vlayout->addWidget(coordinate_edit.release());
  vlayout->addLayout(hlayout.release());

  return vlayout;
}
}  // namespace

namespace omm
{
PointEdit::PointEdit(PathObject& path_object, PathPoint& point, QWidget* parent)
  : QWidget(parent), m_point(point), m_path(&path_object)
{
  if (m_point.is_selected()) {
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, palette.color(QPalette::AlternateBase));
    setPalette(palette);
    setAutoFillBackground(true);
  }

  auto coupled = std::make_unique<QPushButton>();
  auto left = make_tangent_layout(m_left_tangent_edit, m_mirror_from_right, m_vanish_left);
  auto right = make_tangent_layout(m_right_tangent_edit, m_mirror_from_left, m_vanish_right);

  auto center = std::make_unique<QVBoxLayout>();

  auto position_edit = std::make_unique<CoordinateEdit>();
  m_position_edit = position_edit.get();
  center->addWidget(position_edit.release());

  auto couple_button = std::make_unique<QPushButton>(tr("LNK"));
  couple_button->setCheckable(true);
  couple_button->setChecked(true);
  m_coupled = couple_button.get();
  center->addWidget(couple_button.release());

  auto main = std::make_unique<QHBoxLayout>();
  main->addLayout(left.release());
  main->addLayout(center.release());
  main->addLayout(right.release());

  setLayout(main.release());

  m_left_tangent_edit->set_coordinates(m_point.geometry().left_tangent().to_cartesian());
  m_right_tangent_edit->set_coordinates(m_point.geometry().right_tangent().to_cartesian());
  m_position_edit->set_coordinates(m_point.geometry().position());
  m_position_edit->set_display_mode(DisplayMode::Cartesian);

  connect(m_mirror_from_left, &QPushButton::clicked, this, &PointEdit::mirror_from_left);
  connect(m_mirror_from_right, &QPushButton::clicked, this, &PointEdit::mirror_from_right);
  connect(m_vanish_left, &QPushButton::clicked, [this]() {
    QSignalBlocker m_left(m_left_tangent_edit);
    m_left_tangent_edit->set_magnitude(0.0);
    update_point();
  });
  connect(m_vanish_right, &QPushButton::clicked, [this]() {
    QSignalBlocker m_right(m_right_tangent_edit);
    m_right_tangent_edit->set_magnitude(0.0);
    update_point();
  });

  connect(m_left_tangent_edit,
          &CoordinateEdit::value_changed_val,
          this,
          &PointEdit::set_right_maybe);
  connect(m_right_tangent_edit,
          &CoordinateEdit::value_changed_val,
          this,
          &PointEdit::set_left_maybe);

  {
    connect(m_left_tangent_edit, &CoordinateEdit::value_changed, this, &PointEdit::update_point);
    connect(m_right_tangent_edit, &CoordinateEdit::value_changed, this, &PointEdit::update_point);
    connect(m_position_edit, &CoordinateEdit::value_changed, this, &PointEdit::update_point);
  }
}

void PointEdit::mirror_from_right()
{
  QSignalBlocker b_left(m_left_tangent_edit);
  m_left_tangent_edit->set_coordinates(-m_right_tangent_edit->to_polar());
  update_point();
}

void PointEdit::mirror_from_left()
{
  QSignalBlocker b_right(m_right_tangent_edit);
  m_right_tangent_edit->set_coordinates(-m_left_tangent_edit->to_polar());
  update_point();
}

void PointEdit::set_left_maybe(const PolarCoordinates& old_right, const PolarCoordinates& new_right)
{
  if (m_coupled->isChecked()) {
    const auto old_pos = m_left_tangent_edit->to_polar();
    const auto new_pos = Point::mirror_tangent(old_pos, old_right, new_right);
    QSignalBlocker b_left(m_left_tangent_edit);
    m_left_tangent_edit->set_coordinates(new_pos);
  }
}

void PointEdit::set_right_maybe(const PolarCoordinates& old_left, const PolarCoordinates& new_left)
{
  if (m_coupled->isChecked()) {
    const auto old_pos = m_right_tangent_edit->to_polar();
    const auto new_pos = Point::mirror_tangent(old_pos, old_left, new_left);
    QSignalBlocker b_right(m_right_tangent_edit);
    m_right_tangent_edit->set_coordinates(new_pos);
  }
}

void PointEdit::update_point()
{
  if (m_path == nullptr || m_path->scene() == nullptr) {
    auto geometry = m_point.geometry();
    geometry.set_left_tangent(m_left_tangent_edit->to_polar());
    geometry.set_position(m_position_edit->to_cartesian());
    geometry.set_right_tangent(m_right_tangent_edit->to_polar());
    m_point.set_geometry(geometry);
  } else {
    std::map<PathPoint*, Point> map;
    Point new_point(m_position_edit->to_cartesian(),
                    m_left_tangent_edit->to_polar(),
                    m_right_tangent_edit->to_polar());
    map[&m_point] = new_point;
    m_path->scene()->submit<ModifyPointsCommand>(map);
    m_path->update();
  }
}

void PointEdit::set_display_mode(const DisplayMode& display_mode)
{
  m_left_tangent_edit->set_display_mode(display_mode);
  m_right_tangent_edit->set_display_mode(display_mode);
}

}  // namespace omm
