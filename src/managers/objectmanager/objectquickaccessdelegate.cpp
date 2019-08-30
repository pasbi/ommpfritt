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

void draw_cross(QPainter& painter, const QRectF& area, bool is_enabled)
{
  QPen pen;
  if (is_enabled) {
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

void draw_dot(QPainter& painter, const QRectF& area, const omm::Object::Visibility visibility)
{
  static constexpr QMarginsF margins(0.05, 0.05, 0.05, 0.05);

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

}  // namespace

namespace omm
{

ObjectQuickAccessDelegate::ObjectQuickAccessDelegate(ObjectTreeView& view) : m_view(view)
{
}

void ObjectQuickAccessDelegate::
paint(QPainter *painter, const QStyleOptionViewItem &, const QModelIndex &index) const
{
  static constexpr QMarginsF margins(0.1, 0.1, 0.1, 0.1);

  const auto& object = m_view.model()->item_at(index);

  painter->save();
  const auto rect = m_view.visualRect(index);
  painter->translate(rect.topLeft());
  painter->scale(rect.width(), rect.height());
  draw_cross(*painter, enabled_cross_area - margins, object.is_active());
  draw_dot(*painter, edit_visibility - margins,
           object.property(Object::VISIBILITY_PROPERTY_KEY)->value<Object::Visibility>());
  // draw_dot(*painter, export_visibility);
  painter->restore();
}

QSize ObjectQuickAccessDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
  return QSize(-1, -1);
}

bool ObjectQuickAccessDelegate::on_mouse_button_press(QMouseEvent& event)
{
  const auto index = m_view.indexAt(event.pos());
  const auto pos = to_local(event.pos());
  auto& object = m_view.model()->item_at(index);
  assert(m_macro == nullptr);
  if (enabled_cross_area.contains(pos)) {
    const auto is_active = object.is_active();
    auto& property = *object.property(Object::IS_ACTIVE_PROPERTY_KEY);
    bool new_value = !is_active;
    auto command = std::make_unique<PropertiesCommand<BoolProperty>>(std::set { &property },
                                                                     new_value );
    m_macro = m_view.scene().history.start_macro(QString::fromStdString(command->label()));
    m_view.scene().submit(std::move(command));
    m_active_item = ActiveItem::Activeness;
    m_active_item_value = new_value;
    return true;
  } else if (edit_visibility.contains(pos)) {
    auto& prop = *object.property(Object::VISIBILITY_PROPERTY_KEY);
    const auto v = static_cast<std::size_t>(advance_visibility(prop.value<Object::Visibility>()));
    auto command = std::make_unique<PropertiesCommand<OptionsProperty>>(std::set { &prop }, v);
    m_macro = m_view.scene().history.start_macro(QString::fromStdString(command->label()));
    m_view.scene().submit(std::move(command));
    m_active_item = ActiveItem::Visibility;
    m_active_item_value = v;
    return true;
  } else {
    m_active_item = ActiveItem::None;
    return false;
  }
}

void ObjectQuickAccessDelegate::on_mouse_move(QMouseEvent &event)
{
  const auto pos = to_local(event.pos());
  const auto index = m_view.indexAt(event.pos());
  auto& object = m_view.model()->item_at(index);
  switch (m_active_item) {
  case ActiveItem::Activeness:
    if (enabled_cross_area.contains(pos)) {
      auto& prop = *object.property(Object::IS_ACTIVE_PROPERTY_KEY);
      m_view.scene().submit<PropertiesCommand<BoolProperty>>(std::set { &prop },
                                                             m_active_item_value);
      LINFO << m_active_item_value;
    }
    break;
  case ActiveItem::Visibility:
    if (edit_visibility.contains(pos)) {
      auto& prop = *object.property(Object::VISIBILITY_PROPERTY_KEY);
      m_view.scene().submit<PropertiesCommand<OptionsProperty>>(std::set { &prop },
                                                                m_active_item_value);
      LINFO << m_active_item_value;
    }
    break;
  default:
    ;
  }
}

void ObjectQuickAccessDelegate::on_mouse_release(QMouseEvent &event)
{
  Q_UNUSED(event)
  if (m_macro) {
    m_macro.reset();
  }
  m_active_item = ActiveItem::None;
}

QPointF ObjectQuickAccessDelegate::to_local(const QPoint &view_global) const
{
  const auto index = m_view.indexAt(view_global);
  const auto rect = m_view.visualRect(index);
  auto pos = QPointF(view_global) - rect.topLeft();
  pos.setX(pos.x() / rect.width());
  pos.setY(pos.y() / rect.height());
  return pos;
}

}  // namespace omm
