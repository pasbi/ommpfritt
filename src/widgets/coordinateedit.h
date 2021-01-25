#pragma once

#include "common.h"
#include "geometry/polarcoordinates.h"
#include "geometry/vec2.h"
#include <QWidget>

class QLabel;

namespace omm
{
template<typename> class NumericEdit;

enum class DisplayMode { Polar = 0x1, Cartesian = 0x2, Both = Polar | Cartesian };

class CoordinateEdit : public QWidget
{
  Q_OBJECT
public:
  explicit CoordinateEdit(QWidget* parent = nullptr);

  [[nodiscard]] Vec2f to_cartesian() const;
  [[nodiscard]] PolarCoordinates to_polar() const;

Q_SIGNALS:
  void value_changed_val(const omm::PolarCoordinates& old_val,
                         const omm::PolarCoordinates& new_val);
  void value_changed();

public:
  void set_coordinates(const omm::PolarCoordinates& coordinates);
  void set_coordinates(const omm::Vec2f& coordinates);
  void set_display_mode(const omm::DisplayMode& display_mode);
  void set_magnitude(double magnitude);

private:
  NumericEdit<double>* m_x_edit;
  QLabel* m_cart_label{};
  NumericEdit<double>* m_y_edit;

  NumericEdit<double>* m_arg_edit;
  QLabel* m_polar_label{};
  NumericEdit<double>* m_mag_edit;

  PolarCoordinates m_old_polar_coordinates;

private:
  void update_polar();
  void update_cartesian();
  void emit_value_changed();
};

}  // namespace omm

template<> struct omm::EnableBitMaskOperators<omm::DisplayMode> : std::true_type {
};
