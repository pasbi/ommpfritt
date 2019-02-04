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
    increment(e->angleDelta().y() / 8);
    e->accept();
  }

  void mousePressEvent(QMouseEvent* e) override
  {
    if (e->button() == Qt::LeftButton) { m_mouse_press_pos = e->pos(); }
  }

  void mouseMoveEvent(QMouseEvent* e) override
  {
    if (e->buttons() & Qt::LeftButton) {
      QPoint distance = m_mouse_press_pos - e->pos();
      increment(distance.y());
      QCursor::setPos(mapToGlobal(m_mouse_press_pos));
    }
  }

  void keyPressEvent(QKeyEvent* e) override
  {
    if (e->key() == Qt::Key_Down) { increment(-1); }
    if (e->key() == Qt::Key_Up) { increment(1); }
  }

private:
  value_type m_min;
  value_type m_max;
  value_type m_step = 1;
  QPoint m_mouse_press_pos;

  void increment(double factor)
  {
    set_value(std::max(m_min, std::min<value_type>(m_max, value() + factor * m_step)));
  }

  value_type parse(const std::string& text) const
  {
    std::istringstream sstream(text);
    value_type value;
    sstream >> value;
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
  void set_text(const value_type& value) { setText(QString("%1").arg(value)); }
  static constexpr value_type invalid_value = 0;
};

}  // namespace
