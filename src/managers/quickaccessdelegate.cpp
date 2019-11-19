#include "managers/quickaccessdelegate.h"
#include <QPainter>
#include <QTreeView>
#include <QMouseEvent>

namespace omm
{

QuickAccessDelegate::QuickAccessDelegate(QAbstractItemView& view) : m_view(view)
{
}

void QuickAccessDelegate::
paint(QPainter *painter, const QStyleOptionViewItem &, const QModelIndex &index) const
{
  painter->save();
  painter->setRenderHint(QPainter::HighQualityAntialiasing);
  const auto rect = m_view.visualRect(index);
  painter->translate(rect.topLeft());
  painter->scale(rect.width(), rect.height());
  for (auto& area : m_areas) {
    area->draw(*painter, index);
  }

  // draw_dot(*painter, export_visibility);
  painter->restore();
}

QSize QuickAccessDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
  return QSize(-1, -1);
}

bool QuickAccessDelegate::on_mouse_button_press(QMouseEvent& event)
{
  assert(m_macro == nullptr);
  const auto index = m_view.indexAt(event.pos());
  const QPointF pos = to_local(event.pos(), index);
  for (const auto& area : m_areas) {
    if (area->area.contains(pos)) {
      area->begin(index);
      return true;
    }
  }
  return false;
}

void QuickAccessDelegate::on_mouse_move(QMouseEvent &event)
{
  const auto index = m_view.indexAt(event.pos());
  const QPointF pos = to_local(event.pos(), index);
  for (auto& area : m_areas) {
    if (area->is_active && area->area.contains(pos)) {
      area->perform(index);
    }
  }
}

void QuickAccessDelegate::on_mouse_release(QMouseEvent &event)
{
  Q_UNUSED(event)
  if (m_macro) {
    m_macro.reset();  // calls the dtor of m_macro, which will end the macro.
  }
  for (auto& area : m_areas) {
    area->end();
  }
}

void QuickAccessDelegate::add_area(std::unique_ptr<QuickAccessDelegate::Area> area)
{
  m_areas.push_back(std::move(area));
}

QPointF QuickAccessDelegate::to_local(const QPoint &view_global, const QModelIndex& index) const
{
  assert(m_view.indexAt(view_global) == index);
  const auto rect = m_view.visualRect(index);
  auto pos = QPointF(view_global) - rect.topLeft();
  pos.setX(pos.x() / rect.width());
  pos.setY(pos.y() / rect.height());
  return pos;
}

QuickAccessDelegate::Area::Area(const QRectF& area) : area(area)
{

}

}  // namespace omm
