#pragma once

#include <QWidget>
#include "geometry/vec2.h"
#include "geometry/polarcoordinates.h"
#include "common.h"

class QLabel;

namespace omm
{

template<typename> class NumericEdit;

enum class DisplayMode { Polar = 0x1, Cartesian = 0x2, Both = Polar | Cartesian };

class CoordinateEdit : public QWidget
{
  Q_OBJECT
public:

  explicit CoordinateEdit(QWidget *parent = nullptr);

  Vec2f to_cartesian() const;
  PolarCoordinates to_polar() const;

Q_SIGNALS:
  void value_changed(const PolarCoordinates& old_val, const PolarCoordinates& new_val);
  void value_changed();

public Q_SLOTS:
  void set_coordinates(const PolarCoordinates &coordinates);
  void set_coordinates(const Vec2f& coordinates);
  void set_display_mode(const DisplayMode& display_mode);
  void set_magnitude(const double magnitude);

private:
  NumericEdit<double>* m_x_edit;
  QLabel* m_cart_label;
  NumericEdit<double>* m_y_edit;

  NumericEdit<double>* m_arg_edit;
  QLabel* m_polar_label;
  NumericEdit<double>* m_mag_edit;

  PolarCoordinates m_old_polar_coordinates;

private Q_SLOTS:
  void update_polar();
  void update_cartesian();
  void emit_value_changed();
};

}  // namespace omm

template<> struct omm::EnableBitMaskOperators<omm::DisplayMode> : std::true_type {};
