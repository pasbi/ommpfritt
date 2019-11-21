#include "managers/curvemanager/curvemanagerquickaccessdelegate.h"
#include <QPainter>

namespace
{

class VisibilityArea : public omm::QuickAccessDelegate::Area
{
public:
  explicit VisibilityArea(const QRectF& area);
  void draw(QPainter& painter, const QModelIndex& index) override;
  void begin(const QModelIndex& index) override;
  void perform(const QModelIndex& index) override;
  void end() override;
};

}  // namespace

namespace omm
{

CurveManagerQuickAccessDelegate::CurveManagerQuickAccessDelegate(QAbstractItemView& view)
  : QuickAccessDelegate(view)
{
  add_area(std::make_unique<VisibilityArea>(QRectF(0.0, 0.0, 1.0, 1.0)));
}

}  // namespace omm

VisibilityArea::VisibilityArea(const QRectF& area) : omm::QuickAccessDelegate::Area(area)
{

}

void VisibilityArea::draw(QPainter& painter, const QModelIndex& index)
{
  painter.fillRect(area, Qt::yellow);
}

void VisibilityArea::begin(const QModelIndex& index)
{

}

void VisibilityArea::perform(const QModelIndex& index)
{

}

void VisibilityArea::end()
{

}
