#pragma once

#include <QLineEdit>
#include <sstream>
#include <numeric>
#include <cmath>
#include <QWheelEvent>
#include <glog/logging.h>

namespace omm
{

template<typename ValueType>
class NumericEdit : public QLineEdit
{
public:
  using value_type = ValueType;
  using on_value_changed_t = std::function<void(value_type)>;
  NumericEdit(const on_value_changed_t& on_value_changed, QWidget* parent = nullptr)
    : QLineEdit(parent)
    , m_on_value_changed(on_value_changed)
  {
    setContextMenuPolicy(Qt::NoContextMenu);
    if constexpr (std::numeric_limits<value_type>::has_infinity) {
      m_min = -std::numeric_limits<value_type>::infinity();
      m_max =  std::numeric_limits<value_type>::infinity();
    } else {
      m_min =  std::numeric_limits<value_type>::lowest();
      m_max =  std::numeric_limits<value_type>::max();
    }

    connect(this, &QLineEdit::textEdited, [this](const QString& text) {
      const auto value = this->parse(text.toStdString());
      if (!std::isnan(value)) { m_on_value_changed(value); }
    });

    set_text(invalid_value);
  }

  void set_range(value_type min, value_type max)
  {
    assert(min < max);
    m_min = min;
    m_max = max;
  }

  void set_step(value_type step)
  {
    assert(step > 0);
    m_step = step;
  }

  void set_multiplier(double multiplier) { m_multiplier = multiplier; }

  void set_value(const value_type& value)
  {
    if (value != this->value()) {
      if (std::isnan(value)) {
        set_invalid_value();
      } else {
        set_text(value);
        m_on_value_changed(value);
      }
    } else if (value == invalid_value) {
      set_text(invalid_value);
    }
  }

  void set_invalid_value() { setText("< invalid >"); }
  value_type value() const { return parse(text().toStdString()); }

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
  value_type m_min;
  value_type m_max;
  value_type m_step = 1;
  double m_multiplier = 1.0;
  QPoint m_mouse_press_pos;

  void increment(double factor)
  {
    const auto increment = factor * m_step / m_multiplier;
    set_value(std::max(m_min, std::min<value_type>(m_max, value() + increment)));
  }

  value_type parse(const std::string& text) const
  {
    std::istringstream sstream(text);
    value_type value;
    sstream >> value;
    value /= m_multiplier;
    if (sstream) {
      return value;
    } else {
      if (m_min < 0 && 0 < m_max) {
        return 0;
      } else {
        return m_min;
      }
    }
  }

  const on_value_changed_t m_on_value_changed;
  void set_text(const value_type& value) { setText(QString("%1").arg(m_multiplier * value)); }
  static constexpr value_type invalid_value = 0;
};

}  // namespace
