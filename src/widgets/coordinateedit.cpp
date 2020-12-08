#include "coordinateedit.h"
#include "geometry/polarcoordinates.h"
#include "widgets/numericedit.h"
#include <QGridLayout>
#include <QLabel>

namespace
{
auto make_label(const QString& text, QLabel*& label_ref)
{
  auto label = std::make_unique<QLabel>(text);
  label_ref = label.get();
  label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  return label;
}

}  // namespace

namespace omm
{
CoordinateEdit::CoordinateEdit(QWidget* parent) : QWidget(parent)
{
  auto grid_layout = std::make_unique<QGridLayout>();

  auto x_edit = std::make_unique<NumericEdit<double>>();
  m_x_edit = x_edit.get();
  grid_layout->addWidget(x_edit.release(), 0, 0);

  grid_layout->addWidget(make_label(",", m_cart_label).release(), 0, 1);

  auto y_edit = std::make_unique<NumericEdit<double>>();
  m_y_edit = y_edit.get();
  grid_layout->addWidget(y_edit.release(), 0, 2);

  auto arg_edit = std::make_unique<NumericEdit<double>>();
  arg_edit->set_multiplier(M_180_PI);
  m_arg_edit = arg_edit.get();
  grid_layout->addWidget(arg_edit.release(), 1, 0);

  grid_layout->addWidget(make_label("∠", m_polar_label).release(), 1, 1);

  auto mag_edit = std::make_unique<NumericEdit<double>>();
  m_mag_edit = mag_edit.get();
  m_mag_edit->set_lower(0.0);
  grid_layout->addWidget(mag_edit.release(), 1, 2);

  setLayout(grid_layout.release());

  connect(m_x_edit, &QLineEdit::textChanged, this, &CoordinateEdit::update_polar);
  connect(m_y_edit, &QLineEdit::textChanged, this, &CoordinateEdit::update_polar);
  connect(m_arg_edit, &QLineEdit::textChanged, this, &CoordinateEdit::update_cartesian);
  connect(m_mag_edit, &QLineEdit::textChanged, this, &CoordinateEdit::update_cartesian);

  connect(m_x_edit, &QLineEdit::textChanged, this, &CoordinateEdit::emit_value_changed);
  connect(m_y_edit, &QLineEdit::textChanged, this, &CoordinateEdit::emit_value_changed);
  connect(m_arg_edit, &QLineEdit::textChanged, this, &CoordinateEdit::emit_value_changed);
  connect(m_mag_edit, &QLineEdit::textChanged, this, &CoordinateEdit::emit_value_changed);
  update();

  set_display_mode(DisplayMode::Both);
}

Vec2f CoordinateEdit::to_cartesian() const
{
  return Vec2f(m_x_edit->value(), m_y_edit->value());
}

PolarCoordinates CoordinateEdit::to_polar() const
{
  return PolarCoordinates(m_arg_edit->value(), m_mag_edit->value());
}

void CoordinateEdit::set_coordinates(const Vec2f& coordinates)
{
  m_x_edit->set_value(coordinates.x);
  m_y_edit->set_value(coordinates.y);
  update_polar();
}

void CoordinateEdit::set_coordinates(const PolarCoordinates& coordinates)
{
  m_arg_edit->set_value(coordinates.argument);
  m_mag_edit->set_value(coordinates.magnitude);
  update_cartesian();
}

void CoordinateEdit::update_polar()
{
  const auto pc = PolarCoordinates(to_cartesian());
  QSignalBlocker b_arg(m_arg_edit);
  QSignalBlocker b_mag(m_mag_edit);
  m_arg_edit->set_value(pc.argument);
  m_mag_edit->set_value(pc.magnitude);
}

void CoordinateEdit::update_cartesian()
{
  const auto cartesian = to_polar().to_cartesian();
  QSignalBlocker b_x(m_x_edit);
  QSignalBlocker b_y(m_y_edit);
  m_x_edit->set_value(cartesian.x);
  m_y_edit->set_value(cartesian.y);
}

void CoordinateEdit::emit_value_changed()
{
  const auto new_polar_coordinates = to_polar();
  Q_EMIT value_changed_val(m_old_polar_coordinates, new_polar_coordinates);
  Q_EMIT value_changed();
  m_old_polar_coordinates = new_polar_coordinates;
}

void CoordinateEdit::set_display_mode(const DisplayMode& display_mode)
{
  m_x_edit->setVisible(!!(display_mode & DisplayMode::Cartesian));
  m_y_edit->setVisible(!!(display_mode & DisplayMode::Cartesian));
  m_cart_label->setVisible(!!(display_mode & DisplayMode::Cartesian));
  m_arg_edit->setVisible(!!(display_mode & DisplayMode::Polar));
  m_mag_edit->setVisible(!!(display_mode & DisplayMode::Polar));
  m_polar_label->setVisible(!!(display_mode & DisplayMode::Polar));
}

void CoordinateEdit::set_magnitude(const double magnitude)
{
  m_mag_edit->set_value(magnitude);
  update_cartesian();
}

}  // namespace omm
