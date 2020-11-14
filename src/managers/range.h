#pragma once

#include "common.h"
#include <QWidget>
#include <vector>

namespace omm
{
struct Range {
  enum class Options { Default = 0, Mirror = 1 };
  Range(double begin, double end, Options options = Options::Default);
  double begin;
  double end;
  virtual int pixel_range() const = 0;
  double pixel_to_unit(double pixel) const;
  double unit_to_pixel(double unit) const;
  double unit_to_normalized(double unit) const;
  double normalized_to_unit(double normalized) const;
  std::vector<double> scale(double spacing) const;
  double spacing(double distance) const;
  void pan(double d);
  void zoom(double origin, double amount, double min_upp, double max_upp);

protected:
  const bool mirror;
};

template<Qt::Orientation orientation> struct WidgetRange : public Range {
  WidgetRange(double begin, double end, QWidget& widget, Options options = Options::Default)
      : Range(begin, end, options), m_widget(widget)
  {
  }
  int pixel_range() const override
  {
    if constexpr (orientation == Qt::Horizontal) {
      return m_widget.width();
    } else {
      return m_widget.height();
    }
  }

private:
  QWidget& m_widget;
};

struct WidgetRange2D {
  WidgetRange2D(QPointF begin,
                QPointF end,
                QWidget& widget,
                Range::Options h_options = Range::Options::Default,
                Range::Options v_options = Range::Options::Default);
  QPointF begin() const;
  void set_begin(const QPointF& begin);
  QPointF end() const;
  void set_end(const QPointF& end);

  QPointF pixel_to_unit(const QPointF& pixel) const;
  QPointF unit_to_pixel(const QPointF& unit) const;
  QPointF unit_to_normalized(const QPointF& normalized) const;
  QPointF normalized_to_unit(const QPointF& unit) const;
  void pan(const QPointF& d);
  void zoom(const QPointF& origin,
            const QPointF& amount,
            const QPointF& min_upp,
            const QPointF& max_upp);

  WidgetRange<Qt::Horizontal> h_range;
  WidgetRange<Qt::Vertical> v_range;
};

}  // namespace omm

template<> struct omm::EnableBitMaskOperators<omm::Range::Options> : std::true_type {
};
