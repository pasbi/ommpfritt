#include "coordinateedit.h"
#include "widgets/numericedit.h"
#include <QGridLayout>
#include <QLabel>
#include "geometry/polarcoordinates.h"

namespace
{

auto make_label(const QString& text, QLabel*& label_ref)
{
  auto label = std::make_unique<QLabel>(text);
  label_ref = label.get();
  label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  return label;
}

}

namespace omm
{

CoordinateEdit::CoordinateEdit(QWidget *parent) : QWidget(parent)
{
  auto grid_layout = std::make_unique<QGridLayout>();

  static const auto noop = [](auto&&...){};
  auto x_edit = std::make_unique<NumericEdit<double>>(noop);
  m_x_edit = x_edit.get();
  grid_layout->addWidget(x_edit.release(), 0, 0);

  grid_layout->addWidget(make_label(",", m_cart_label).release(), 0, 1);

  auto y_edit = std::make_unique<NumericEdit<double>>(noop);
  m_y_edit = y_edit.get();
  grid_layout->addWidget(y_edit.release(), 0, 2);

  auto arg_edit = std::make_unique<NumericEdit<double>>(noop);
  m_arg_edit = arg_edit.get();
  grid_layout->addWidget(arg_edit.release(), 1, 0);

  grid_layout->addWidget(make_label("∠", m_polar_label).release(), 1, 1);

  auto mag_edit = std::make_unique<NumericEdit<double>>(noop);
  m_mag_edit = mag_edit.get();
  m_mag_edit->set_lower(0.0);
  grid_layout->addWidget(mag_edit.release(), 1, 2);

  setLayout(grid_layout.release());

  connect(m_x_edit, SIGNAL(textChanged(QString)), this, SLOT(update_polar()));
  connect(m_y_edit, SIGNAL(textChanged(QString)), this, SLOT(update_polar()));
  connect(m_arg_edit, SIGNAL(textChanged(QString)), this, SLOT(update_cartesian()));
  connect(m_mag_edit, SIGNAL(textChanged(QString)), this, SLOT(update_cartesian()));

  connect(m_x_edit, SIGNAL(textChanged(QString)), this, SLOT(emit_value_changed_from_cartesian()));
  connect(m_y_edit, SIGNAL(textChanged(QString)), this, SLOT(emit_value_changed_from_cartesian()));
  connect(m_arg_edit, SIGNAL(textChanged(QString)), this, SLOT(emit_value_changed_from_polar()));
  connect(m_mag_edit, SIGNAL(textChanged(QString)), this, SLOT(emit_value_changed_from_polar()));

}

Vec2f CoordinateEdit::to_cartesian() const
{
  const auto p = value_from_polar();
#ifndef NDEBUG
  const auto c = value_from_cartesian();
  LINFO << c << " " << p.to_cartesian();
  assert((c - p.to_cartesian()).max_norm() < 0.01);
#endif  // NDEBUG
  return c;
}

PolarCoordinates CoordinateEdit::to_polar() const
{
  const auto c = value_from_cartesian();
#ifndef NDEBUG
  const auto p = value_from_polar();
  LINFO << c << " " << p.to_cartesian() << " " << (c - p.to_cartesian()).max_norm();
  assert((c - p.to_cartesian()).max_norm() < 0.01);  //
#endif  // NDEBUG
  return p;
}

void CoordinateEdit::set_coordinates(const Vec2f &coordinates)
{
  m_x_edit->set_value(coordinates.x);
  m_y_edit->set_value(coordinates.y);
  update_polar();
}

void CoordinateEdit::set_coordinates(const PolarCoordinates& coordinates)
{
  m_arg_edit->set_value(coordinates.argument * 180 * M_1_PI);
  m_mag_edit->set_value(coordinates.magnitude);
  update_cartesian();
}

void CoordinateEdit::update_polar()
{
  const auto pc = PolarCoordinates(value_from_cartesian());
  QSignalBlocker b_arg(m_arg_edit);
  QSignalBlocker b_mag(m_mag_edit);
  m_arg_edit->set_value(pc.argument * 180 * M_1_PI);
  m_mag_edit->set_value(pc.magnitude);
}

void CoordinateEdit::update_cartesian()
{
  const auto cartesian = value_from_polar().to_cartesian();
  QSignalBlocker b_x(m_x_edit);
  QSignalBlocker b_y(m_y_edit);
  m_x_edit->set_value(cartesian.x);
  m_y_edit->set_value(cartesian.y);
}

PolarCoordinates CoordinateEdit::value_from_polar() const
{
  return PolarCoordinates(m_arg_edit->value() / 180.0 * M_PI, m_mag_edit->value());
}

Vec2f CoordinateEdit::value_from_cartesian() const
{
  return Vec2f(m_x_edit->value(), m_y_edit->value());
}

void CoordinateEdit::emit_value_changed_from_polar()
{
  const auto new_polar_coordinates = value_from_polar();
  Q_EMIT value_changed(m_old_polar_coordinates, new_polar_coordinates);
  Q_EMIT value_changed();
  m_old_polar_coordinates = new_polar_coordinates;
}

void CoordinateEdit::emit_value_changed_from_cartesian()
{
  const auto new_polar_coordinates = value_from_polar();
  Q_EMIT value_changed(m_old_polar_coordinates, new_polar_coordinates);
  Q_EMIT value_changed();
  m_old_polar_coordinates = new_polar_coordinates;
}

void CoordinateEdit::set_mode_cartesian()
{
  m_x_edit->setVisible(true);
  m_y_edit->setVisible(true);
  m_cart_label->setVisible(true);
  m_arg_edit->setVisible(false);
  m_mag_edit->setVisible(false);
  m_polar_label->setVisible(false);
}

void CoordinateEdit::set_mode_polar()
{
  m_x_edit->setVisible(false);
  m_y_edit->setVisible(false);
  m_cart_label->setVisible(false);
  m_arg_edit->setVisible(true);
  m_mag_edit->setVisible(true);
  m_polar_label->setVisible(true);
}

void CoordinateEdit::set_mode_both()
{
  m_x_edit->setVisible(true);
  m_y_edit->setVisible(true);
  m_cart_label->setVisible(true);
  m_arg_edit->setVisible(true);
  m_mag_edit->setVisible(true);
  m_polar_label->setVisible(true);
}

}  // namespace omm

