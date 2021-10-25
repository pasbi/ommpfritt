#include "managers/objectmanager/objectquickaccessdelegate.h"
#include "commands/propertycommand.h"
#include "main/application.h"
#include "managers/objectmanager/objecttreeview.h"
#include "preferences/uicolors.h"
#include "properties/boolproperty.h"
#include "properties/optionproperty.h"
#include "scene/history/historymodel.h"
#include "scene/history/macro.h"
#include "scene/scene.h"
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>

namespace
{
using namespace omm;

constexpr QSizeF half_size_hv{0.5, 0.5};
constexpr QSizeF half_size_v{0.5, 1.0};
constexpr QPointF pos_upper_visibility{0.5, 0.0};
constexpr QPointF pos_lower_visibility{0.5, 0.5};

class PropertyArea : public ObjectQuickAccessDelegate::Area
{
public:
  PropertyArea(const QRectF& area, ObjectTreeView& view, const QString& property_key);
  bool draw_active = false;
  ObjectTreeView& view;
  [[nodiscard]] Property& property(const QModelIndex& index) const;
  virtual std::unique_ptr<Command> make_command(const QModelIndex& index, bool update_cache) = 0;
  void begin(const QModelIndex& index, QMouseEvent&) override;
  void end() override;
  void perform(const QModelIndex& index, QMouseEvent&) override;

private:
  const QString m_property_key;
  std::unique_ptr<Command> m_command_on_hold;
  std::unique_ptr<Macro> m_macro;
};

class VisibilityPropertyArea : public PropertyArea
{
public:
  explicit VisibilityPropertyArea(ObjectTreeView& view, const QRectF& rect, const QString& key);
  void draw(QPainter& painter, const QModelIndex& index, const QRectF& rect) override;

protected:
  std::unique_ptr<Command> make_command(const QModelIndex& index, bool update_cache) override;

private:
  Object::Visibility m_new_value = Object::Visibility::Default;
};

class IsEnabledPropertyArea : public PropertyArea
{
public:
  explicit IsEnabledPropertyArea(ObjectTreeView& view);
  void draw(QPainter& painter, const QModelIndex& index, const QRectF& rect) override;

protected:
  std::unique_ptr<Command> make_command(const QModelIndex& index, bool update_cache) override;

private:
  bool m_new_value = false;
};

Object::Visibility advance_visibility(Object::Visibility visibility)
{
  switch (visibility) {
  case Object::Visibility::Default:
    return Object::Visibility::Hidden;
  case Object::Visibility::Hidden:
    return Object::Visibility::Visible;
  case Object::Visibility::Visible:
    return Object::Visibility::Default;
  }
  Q_UNREACHABLE();
}

PropertyArea::PropertyArea(const QRectF& area, ObjectTreeView& view, const QString& property_key)
    : ObjectQuickAccessDelegate::Area(area), view(view), m_property_key(property_key)
{
}

Property& PropertyArea::property(const QModelIndex& index) const
{
  return *view.model()->item_at(index).property(m_property_key);
}

void PropertyArea::begin(const QModelIndex& index, QMouseEvent&)
{
  is_active = true;
  Scene& scene = *Application::instance().scene;

  // unfortunately, commands cannot be copied. Creating it twice should be fine...
  m_command_on_hold = make_command(index, true);
  auto command = make_command(index, true);
  scene.submit(std::move(command));
}

void PropertyArea::end()
{
  is_active = false;
  m_macro.reset();
}

void PropertyArea::perform(const QModelIndex& index, QMouseEvent&)
{
  auto command = make_command(index, false);
  if (command != nullptr) {
    Scene& scene = *Application::instance().scene;
    // if the macro has not yet been started, start it now.
    if (m_macro == nullptr) {
      // move the command that was issued on click inside the macro.
      scene.history().undo();  // remove the single command
      const QString label = m_command_on_hold->actionText();
      m_macro = scene.history().start_macro(label);

      // commit a copy of the removed single command again, now within the macro.
      scene.submit(std::move(m_command_on_hold));
    }

    // commit the actual command
    scene.submit(std::move(command));
  }
}

VisibilityPropertyArea::VisibilityPropertyArea(ObjectTreeView& view,
                                               const QRectF& rect,
                                               const QString& key)
    : PropertyArea(rect, view, key)
{
}

void VisibilityPropertyArea::draw(QPainter& painter, const QModelIndex& index, const QRectF& rect)
{
  static constexpr QMarginsF margins(0.05, 0.05, 0.05, 0.05);
  const auto visibility = property(index).value<Object::Visibility>();

  painter.save();
  painter.translate(rect.topLeft());
  painter.scale(rect.width(), rect.height());

  QPen pen;
  pen.setWidthF(2.0);
  pen.setCosmetic(true);
  pen.setColor(ui_color(view, "ObjectManager", "visibility outline"));
  painter.setPen(pen);
  painter.setBrush([visibility, this]() {
    switch (visibility) {
    case Object::Visibility::Default:
      return ui_color(view, "ObjectManager", "default fill");
    case Object::Visibility::Hidden:
      return ui_color(view, "ObjectManager", "invisible fill");
    case Object::Visibility::Visible:
      return ui_color(view, "ObjectManager", "visible fill");
    }
    Q_UNREACHABLE();
    return QColor();
  }());
  painter.drawEllipse(area - margins);
  painter.restore();
}

std::unique_ptr<Command> VisibilityPropertyArea::make_command(const QModelIndex& index,
                                                              bool update_cache)
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
    return std::make_unique<PropertiesCommand<OptionProperty>>(std::set{&property}, value_s);
  }
}

IsEnabledPropertyArea::IsEnabledPropertyArea(ObjectTreeView& view)
    : PropertyArea(QRectF(QPointF(0.0, 0.0), half_size_v), view, Object::IS_ACTIVE_PROPERTY_KEY)
{
}

void IsEnabledPropertyArea::draw(QPainter& painter, const QModelIndex& index, const QRectF& rect)
{
  painter.save();
  painter.translate(rect.topLeft());
  painter.scale(rect.width(), rect.height());

  QPen pen;
  if (property(index).value<bool>()) {
    pen.setColor(ui_color(view, "ObjectManager", "enabled"));
  } else {
    pen.setColor(ui_color(view, "ObjectManager", "disabled"));
  }
  const double width = 0.08;
  pen.setWidthF(width);
  pen.setCosmetic(false);
  pen.setCapStyle(Qt::RoundCap);

  {
    const QMarginsF margins = QMarginsF(width, width, width, width);
    const auto net_area = area - margins;
    painter.setPen(pen);
    painter.drawLine(net_area.topLeft(), net_area.bottomRight());
    painter.drawLine(net_area.topRight(), net_area.bottomLeft());
  }
  painter.restore();
}

std::unique_ptr<Command> IsEnabledPropertyArea::make_command(const QModelIndex& index,
                                                             bool update_cache)
{
  auto& property = this->property(index);
  const auto old_value = property.value<bool>();
  if (update_cache) {
    m_new_value = !old_value;
  }
  if (m_new_value == old_value) {
    return nullptr;
  } else {
    return std::make_unique<PropertiesCommand<BoolProperty>>(std::set{&property}, m_new_value);
  }
}

}  // namespace

namespace omm
{
ObjectQuickAccessDelegate::ObjectQuickAccessDelegate(QAbstractItemView& view)
    : QuickAccessDelegate(view)
{
  auto& otv = dynamic_cast<ObjectTreeView&>(view);
  add_area(std::make_unique<IsEnabledPropertyArea>(otv));
  using VPA = VisibilityPropertyArea;
  add_area(std::make_unique<VPA>(otv,
                                 QRectF(pos_lower_visibility, half_size_hv),
                                 Object::VISIBILITY_PROPERTY_KEY));
  add_area(std::make_unique<VPA>(otv,
                                 QRectF(pos_upper_visibility, half_size_hv),
                                 Object::VIEWPORT_VISIBILITY_PROPERTY_KEY));
}

}  // namespace omm
