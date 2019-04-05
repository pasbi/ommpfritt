#include "widgets/pointedit.h"
#include "widgets/coordinateedit.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <memory>
#include <QPushButton>
#include "scene/scene.h"
#include "commands/modifypointscommand.h"

auto make_tangent_layout( omm::CoordinateEdit*& coordinate_edit_ref,
                          QPushButton*& mirror_button_ref,
                          QPushButton*& vanish_button_ref )
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
  mirror_button.release();

  auto vlayout = std::make_unique<QVBoxLayout>();
  vlayout->addWidget(coordinate_edit.release());
  vlayout->addLayout(hlayout.release());

  return vlayout;
}

namespace omm
{

PointEdit::PointEdit(Point& point, Path* path, QWidget* parent)
  : QWidget(parent), m_point(point), m_path(path)
{
  auto coupled = std::make_unique<QPushButton>();

  auto left = make_tangent_layout(m_left_tangent_edit, m_mirror_from_right, m_vanish_left);
  auto right = make_tangent_layout(m_right_tangent_edit, m_mirror_from_left, m_vanish_right);

  auto center = std::make_unique<QVBoxLayout>();

  auto position_edit = std::make_unique<CoordinateEdit>();
  m_position_edit = position_edit.get();
  m_position_edit->set_mode_cartesian();
  center->addWidget(position_edit.release());

  auto couple_button = std::make_unique<QPushButton>("X");
  couple_button->setCheckable(true);
  couple_button->setChecked(true);
  m_coupled = couple_button.get();
  center->addWidget(couple_button.release());

  auto main = std::make_unique<QHBoxLayout>();
  main->addLayout(left.release());
  main->addLayout(center.release());
  main->addLayout(right.release());

  setLayout(main.release());

  m_left_tangent_edit->set_coordinates(point.left_tangent.to_cartesian());
  m_right_tangent_edit->set_coordinates(point.right_tangent.to_cartesian());
  m_position_edit->set_coordinates(point.position);

  connect(m_mirror_from_left, SIGNAL(clicked()), this, SLOT(mirror_from_left()));
  connect(m_mirror_from_right, SIGNAL(clicked()), this, SLOT(mirror_from_right()));
  connect(m_vanish_left, &QPushButton::clicked, [this]() {
    QSignalBlocker m_left(m_left_tangent_edit);
    m_left_tangent_edit->set_coordinates(Vec2f::o());
  });
  connect(m_vanish_right, &QPushButton::clicked, [this]() {
    QSignalBlocker m_right(m_right_tangent_edit);
    m_right_tangent_edit->set_coordinates(Vec2f::o());
  });
  connect( m_left_tangent_edit,
           SIGNAL(value_changed(const PolarCoordinates&, const PolarCoordinates&)),
           this, SLOT(set_right_maybe(const PolarCoordinates&, const PolarCoordinates&)) );
  connect( m_right_tangent_edit,
           SIGNAL(value_changed(const PolarCoordinates&, const PolarCoordinates&)),
           this, SLOT(set_left_maybe(const PolarCoordinates&, const PolarCoordinates&)) );

  {
    connect(m_left_tangent_edit, SIGNAL(value_changed()), this, SLOT(update_point()));
    connect(m_right_tangent_edit, SIGNAL(value_changed()), this, SLOT(update_point()));
    connect(m_position_edit, SIGNAL(value_changed()), this, SLOT(update_point()));
  }
}

void PointEdit::mirror_from_right()
{
  QSignalBlocker b_left(m_left_tangent_edit);
  m_left_tangent_edit->set_coordinates(-m_right_tangent_edit->to_polar());
}

void PointEdit::mirror_from_left()
{
  QSignalBlocker b_right(m_right_tangent_edit);
  m_right_tangent_edit->set_coordinates(-m_left_tangent_edit->to_polar());
}

void
PointEdit::set_left_maybe(const PolarCoordinates& old_right, const PolarCoordinates& new_right)
{
  if (m_coupled->isChecked()) {
    const auto old_pos = m_left_tangent_edit->to_polar();
    const auto new_pos = Point::mirror_tangent(old_pos, old_right, new_right);
    QSignalBlocker b_left(m_left_tangent_edit);
    m_left_tangent_edit->set_coordinates(new_pos);
  }
}

void
PointEdit::set_right_maybe(const PolarCoordinates& old_left, const PolarCoordinates& new_left)
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
    m_point.left_tangent = m_left_tangent_edit->to_polar();
    m_point.position = m_position_edit->to_cartesian();
    m_point.right_tangent = m_right_tangent_edit->to_polar();
  } else {
    ModifyPointsCommand::map_type map;
    map[m_path][&m_point] = Point( m_position_edit->to_cartesian(),
                                   m_left_tangent_edit->to_polar(),
                                   m_right_tangent_edit->to_polar() );
    m_path->scene()->submit<ModifyPointsCommand>(map);
  }
}

}  // namespace omm
