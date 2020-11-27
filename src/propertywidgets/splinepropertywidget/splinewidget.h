#pragma once

#include "cachedgetter.h"
#include "propertywidgets/multivalueedit.h"
#include "splinetype.h"
#include <QWidget>

namespace omm
{
class SplineWidget
    : public QWidget
    , public MultiValueEdit<SplineType>
{
  Q_OBJECT

public:
  SplineWidget(QWidget* parent = nullptr);
  void set_value(const value_type& spline) override;
  [[nodiscard]] SplineType value() const override
  {
    return m_spline;
  }

protected:
  void paintEvent(QPaintEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void set_inconsistent_value() override;

Q_SIGNALS:
  void value_changed(const omm::SplineType& spline);

private:
  SplineType m_spline;

  void draw_spline(QPainter& painter);

  [[nodiscard]] QTransform transform() const;
  SplineType::ControlPoint m_grabbed_knot;
  SplineType::ControlPoint knot_at(const QPoint& pos);
};

}  // namespace omm
