#include "managers/objectmanager/objectquickaccessdelegate.h"
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include "managers/objectmanager/objecttreeview.h"
#include <QDebug>
#include "scene/scene.h"
#include "commands/propertycommand.h"
#include "properties/boolproperty.h"
#include "properties/optionsproperty.h"
#include "scene/history/historymodel.h"

namespace
{

omm::Object::Visibility advance_visibility(omm::Object::Visibility visibility)
{
  switch (visibility) {
  case omm::Object::Visibility::Default: return omm::Object::Visibility::Hidden;
  case omm::Object::Visibility::Hidden: return omm::Object::Visibility::Visible;
  case omm::Object::Visibility::Visible: return omm::Object::Visibility::Default;
  }
  Q_UNREACHABLE();
}

}  // namespace

namespace omm
{

PropertyArea::PropertyArea(const QRectF& area, ObjectTreeView& view, const std::string &property_key)
  : area(area), view(view), m_property_key(property_key)
{
}

Property &PropertyArea::property(const QModelIndex &index) const
{
  return *view.model()->item_at(index).property(m_property_key);
}

VisibilityPropertyArea::VisibilityPropertyArea(ObjectTreeView& view)
  : PropertyArea(QRectF(QPointF(0.5, 0.5), QSizeF(0.5, 0.5)), view, Object::VISIBILITY_PROPERTY_KEY)
{
}

void VisibilityPropertyArea::draw(QPainter &painter, const QModelIndex& index)
{
  static constexpr QMarginsF margins(0.05, 0.05, 0.05, 0.05);
  const auto visibility = property(index).value<Object::Visibility>();

  painter.save();
  QPen pen;
  pen.setWidthF(1.0);
  pen.setCosmetic(true);
  pen.setColor(Qt::black);
  painter.setPen(pen);
  painter.drawEllipse(area);
  painter.setBrush([visibility]() {
    switch (visibility) {
    case omm::Object::Visibility::Default: return QColor(Qt::transparent);
    case omm::Object::Visibility::Hidden: return QColor(Qt::red);
    case omm::Object::Visibility::Visible: return QColor(Qt::green);
    }
    Q_UNREACHABLE();
    return QColor();
  }());
  painter.drawEllipse(area - margins);
  painter.restore();
}

std::unique_ptr<Command>
VisibilityPropertyArea::make_command(const QModelIndex &index, bool update_cache)
{
  auto& property = this->property(index);
  const auto old_value = property.value<Object::Visibility>();
  if (update_cache) {
    m_new_value = advance_visibility(old_value);
  }
  if (m_new_value == old_value) {
    return nullptr;
  } else {
    const auto value_s = static_cast<std::size_t>(m_new_value);
    return std::make_unique<PropertiesCommand<OptionsProperty>>(std::set{ &property }, value_s);
  }
}

IsEnabledPropertyArea::IsEnabledPropertyArea(ObjectTreeView &view)
  : PropertyArea(QRectF(QPointF(0.0, 0.0), QSizeF(0.5, 1.0)), view, Object::IS_ACTIVE_PROPERTY_KEY)
{
}

void IsEnabledPropertyArea::draw(QPainter &painter, const QModelIndex& index)
{
  QPen pen;
  if (property(index).value<bool>()) {
    pen.setColor("#80FF80");
  } else {
    pen.setColor("#FF8080");
  }
  pen.setWidth(4.0);
  pen.setCosmetic(true);
  pen.setCapStyle(Qt::RoundCap);

  const auto rect = area - QMarginsF(0.02, 0.02, 0.02, 0.02);

  painter.save();
  painter.setPen(pen);
  painter.drawLine(rect.topLeft(), rect.bottomRight());
  painter.drawLine(rect.topRight(), rect.bottomLeft());
  painter.restore();
}

std::unique_ptr<Command>
IsEnabledPropertyArea::make_command(const QModelIndex &index, bool update_cache)
{
  auto& property = this->property(index);
  const auto old_value = property.value<bool>();
  if (update_cache) {
    m_new_value = !old_value;
  }
  if (m_new_value == old_value) {
    return nullptr;
  } else {
    return std::make_unique<PropertiesCommand<BoolProperty>>(std::set { &property }, m_new_value);
  }
}

ObjectQuickAccessDelegate::ObjectQuickAccessDelegate(ObjectTreeView& view) : m_view(view)
{
  m_areas.push_back(std::make_unique<IsEnabledPropertyArea>(view));
  m_areas.push_back(std::make_unique<VisibilityPropertyArea>(view));
}

void ObjectQuickAccessDelegate::
paint(QPainter *painter, const QStyleOptionViewItem &, const QModelIndex &index) const
{
  painter->save();
  const auto rect = m_view.visualRect(index);
  painter->translate(rect.topLeft());
  painter->scale(rect.width(), rect.height());
  for (auto& area : m_areas) {
    area->draw(*painter, index);
  }

  // draw_dot(*painter, export_visibility);
  painter->restore();
}

QSize ObjectQuickAccessDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
  return QSize(-1, -1);
}

bool ObjectQuickAccessDelegate::on_mouse_button_press(QMouseEvent& event)
{
  assert(m_macro == nullptr);
  const auto index = m_view.indexAt(event.pos());
  const QPointF pos = to_local(event.pos(), index);
  for (const auto& area : m_areas) {
    if (area->area.contains(pos)) {
      area->is_active = true;
      auto command = area->make_command(index, true);
      assert(command != nullptr);  // a click must alter the value, hence command must not be null.
      m_macro = m_view.scene().history.start_macro(QString::fromStdString(command->label()));
      m_view.scene().submit(std::move(command));
      return true;
    }
  }
  return false;
}

void ObjectQuickAccessDelegate::on_mouse_move(QMouseEvent &event)
{
  const auto index = m_view.indexAt(event.pos());
  const QPointF pos = to_local(event.pos(), index);
  for (auto& area : m_areas) {
    if (area->is_active && area->area.contains(pos)) {
      auto command = area->make_command(index, false);
      if (command != nullptr) {
        m_view.scene().submit(std::move(command));
      }
    }
  }
}

void ObjectQuickAccessDelegate::on_mouse_release(QMouseEvent &event)
{
  Q_UNUSED(event)
  if (m_macro) {
    m_macro.reset();
  }
  for (auto& area : m_areas) {
    area->is_active = false;
  }
}

QPointF
ObjectQuickAccessDelegate::to_local(const QPoint &view_global, const QModelIndex& index) const
{
  assert(m_view.indexAt(view_global) == index);
  const auto rect = m_view.visualRect(index);
  auto pos = QPointF(view_global) - rect.topLeft();
  pos.setX(pos.x() / rect.width());
  pos.setY(pos.y() / rect.height());
  return pos;
}

}  // namespace omm
