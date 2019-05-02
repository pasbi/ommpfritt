#pragma once

#include <QLineEdit>
#include <sstream>
#include <numeric>
#include <cmath>
#include <QWheelEvent>
#include <memory>
#include <sstream>
#include <iomanip>
#include <QCoreApplication>
#include "logging.h"

namespace NumericEditDetail
{

// they must be initialized, though they should never be used.
template<typename T> T highest_possible_value = T();
template<typename T> T lowest_possible_value = T();
template<typename T> T smallest_step = T();


// these are the meaningful specializations
template<> constexpr double highest_possible_value<double>
                          = std::numeric_limits<double>::infinity();
template<> constexpr double lowest_possible_value<double>
                          = -std::numeric_limits<double>::infinity();
template<> constexpr double smallest_step<double>
                          = -std::numeric_limits<double>::min();
template<> constexpr int highest_possible_value<int> = std::numeric_limits<int>::max();
template<> constexpr int lowest_possible_value<int> = std::numeric_limits<int>::lowest();
template<> constexpr int smallest_step<int> = 1;

}

namespace omm
{

class AbstractNumericEdit : public QLineEdit
{
  Q_OBJECT
public:
  using QLineEdit::QLineEdit;

Q_SIGNALS:
  void value_changed();
};

template<typename ValueType>
class NumericEdit : public AbstractNumericEdit
{
public:
  const std::string inf = "inf";
  const std::string neg_inf = "-inf";

  using value_type = ValueType;
  NumericEdit(QWidget* parent = nullptr) : AbstractNumericEdit(parent)
  {
    setContextMenuPolicy(Qt::NoContextMenu);

    connect(this, &QLineEdit::textEdited, [this](const QString& text) {
      const auto value = parse(text.toStdString());
      if (value != m_last_value) {
        m_value = value;
        m_last_value = m_value;
        Q_EMIT value_changed();
      }
    });

    connect(this, &QLineEdit::editingFinished, [this]() { set_value(value()); });

    set_text(invalid_value);
  }

  void set_range(const value_type min, const value_type max)
  {
    if (min > max) {
      LERROR << "min is greater than max: " << min << " > " << max;
      m_min = min;
      m_max = min;
    } else {
      m_min = min;
      m_max = max;
    }
  }

  void set_lower(const value_type min)
  {
    if (min > m_max) {
      LERROR << "min is greater than max: " << min << " > " << m_max;
    } else {
      m_min = min;
    }
  }

  void set_upper(const value_type max)
  {
    if (m_min > max) {
      LERROR << "max is greater than min: " << m_min << " > " << max;
    } else {
      m_max = max;
    }
  }

  void set_step(value_type step)
  {
    assert(step > 0);
    m_step = step;
  }

  void set_multiplier(double multiplier) { m_multiplier = multiplier; }

  void set_value(value_type value)
  {
    value = std::clamp(value, m_min, m_max);
    if (std::isnan(value)) {
      set_invalid_value();
    } else {
      if (value != this->value() || !hasFocus()) {
        set_text(value);
        m_value = value;
        if (hasFocus()) { Q_EMIT value_changed(); }
      }
    }
  }

  void set_invalid_value() { setText(QObject::tr("< invalid >", "property")); }
  value_type value() const
  {
    return std::clamp(m_value, m_min, m_max);
  }

protected:
  void wheelEvent(QWheelEvent* e) override
  {
    // TODO
    // e->pixelDelta() is always null for me.
    // e->angleDelta() is (0, +-120) with my logitech mouse
    // e->angleDelta() is (0, n*8) with trackpoint (n being integer)
    // to make behaviour of trackpoint and mouse somewhat consistent, don't use the value
    // of angleDelta but only its direction.
    // curiously, `xev` does not display values on trackpoint/mouse wheel
    //  scroll but only button press events.

    // const auto dy = e->angleDelta().y();
    // if (dy < 0) {
    //   e->accept();
    //   increment(-1); }
    // else if (dy > 0) {
    //   e->accept();
    //   increment(1);
    // }

    QLineEdit::wheelEvent(e);
  }

  void mousePressEvent(QMouseEvent* e) override
  {
    if (e->button() == Qt::RightButton) {
      m_mouse_press_pos = e->pos();
      e->accept();
    } else {
      QLineEdit::mousePressEvent(e);
    }
  }

  void mouseMoveEvent(QMouseEvent* e) override
  {
    if (e->buttons() & Qt::RightButton) {
      QPoint distance = m_mouse_press_pos - e->pos();
      increment(distance.y());
      QCursor::setPos(mapToGlobal(m_mouse_press_pos));
      e->accept();
    } else {
      QLineEdit::mouseMoveEvent(e);
    }
  }

  void keyPressEvent(QKeyEvent* e) override
  {
    if (e->key() == Qt::Key_Down) {
      increment(-1);
      e->accept();
    } else if (e->key() == Qt::Key_Up) {
      increment(1);
      e->accept();
    } else {
      QLineEdit::keyPressEvent(e);
    }
  }

private:
  value_type m_min = NumericEditDetail::lowest_possible_value<value_type>;
  value_type m_max = NumericEditDetail::highest_possible_value<value_type>;
  value_type m_step = 1;
  double m_multiplier = 1.0;
  QPoint m_mouse_press_pos;

  void increment(double factor)
  {
    const auto increment = factor * m_step / m_multiplier;

    // do the range checking in double-domain.
    double new_value = double(this->value()) + double(increment);
    if (new_value > m_max) {
      new_value = m_max;
    } else if (new_value < m_min) {
      new_value = m_min;
    }
    set_value(value_type(new_value));
  }

  value_type parse(const std::string& text) const
  {
    if (text == inf) {
      return NumericEditDetail::highest_possible_value<value_type>;
    } else if (text == neg_inf) {
      return NumericEditDetail::lowest_possible_value<value_type>;
    } else {
      std::istringstream sstream(text);
      value_type value;
      sstream >> value;
      value /= m_multiplier;
      if (sstream) {
        return value;
      } else {
        if (m_min <= 0 && 0 <= m_max) {
          return 0;
        } else {
          return m_min;
        }
      }
    }
  }

  void set_text(const value_type& value)
  {
    m_value = value;
    std::ostringstream ss;
    ss << std::setprecision(3) << std::fixed << value_type(m_multiplier * value);
    const auto new_text = QString::fromStdString(ss.str());
    if (text() != new_text) {
      setText(new_text);
    }
    m_last_value = value;
  }
  static constexpr value_type invalid_value = 0;

public:
  static auto make_range_edits()
  {
    auto min_edit = std::make_unique<NumericEdit<ValueType>>();
    auto max_edit = std::make_unique<NumericEdit<ValueType>>();
    connect(min_edit.get(), &AbstractNumericEdit::value_changed, [&min_edit, &max_edit]() {
      max_edit->set_range(min_edit->value(), NumericEditDetail::highest_possible_value<ValueType>);
    });
    connect(max_edit.get(), &AbstractNumericEdit::value_changed, [&min_edit, &max_edit]() {
      min_edit->set_range(NumericEditDetail::lowest_possible_value<ValueType>, min_edit->value());
    });
    return std::pair(std::move(min_edit), std::move(max_edit));
  }

private:
  ValueType m_last_value;
  ValueType m_value;
};

using IntNumericEdit = NumericEdit<int>;

}  // namespace
