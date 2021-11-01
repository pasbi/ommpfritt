#include "propertywidgets/propertywidget.h"

#include "widgets/animationbutton.h"
#include "scene/scene.h"
#include <QBoxLayout>
#include <QLabel>

namespace omm
{
AbstractPropertyWidget::AbstractPropertyWidget(Scene& scene, const std::set<Property*>& properties)
    : scene(scene), m_properties(properties)
{
  for (Property* property : properties) {
    connect(property,
            &Property::value_changed,
            this,
            &AbstractPropertyWidget::on_property_value_changed);
    connect(property,
            &Property::enabledness_changed,
            this,
            &AbstractPropertyWidget::update_enabledness);
    connect(property,
            &Property::configuration_changed,
            this,
            &AbstractPropertyWidget::update_configuration);
  }
  update_enabledness();
}

void AbstractPropertyWidget::on_property_value_changed(Property*) const
{
  // wait until other properties have updated (important for MultiValueEdit)
  // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
  QTimer::singleShot(1, this, &AbstractPropertyWidget::update_edit);
}

void AbstractPropertyWidget::update_enabledness()
{
  const bool is_enabled
      = std::all_of(m_properties.begin(), m_properties.end(), [](const Property* property) {
          return property->is_enabled();
        });

  setEnabled(is_enabled);
}

void AbstractPropertyWidget::set_widget(std::unique_ptr<QWidget> widget)
{
  auto layout = std::make_unique<QHBoxLayout>();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(widget.release());
  setLayout(layout.release());
}

QString AbstractPropertyWidget::label() const
{
  return Property::get_value<QString>(m_properties, std::mem_fn(&Property::label));
}

void AbstractPropertyWidget::submit_command(std::unique_ptr<Command> command)
{
  scene.submit(std::move(command));
}

AbstractPropertyWidget::LabelLayout::LabelLayout()
{
  setContentsMargins(0, 0, 0, 0);
  auto label = std::make_unique<QLabel>();
  m_label = label.get();
  addWidget(label.release());
}

void AbstractPropertyWidget::LabelLayout::set_label(const QString& label)
{
  m_label->setText(label);
}

void AbstractPropertyWidget::LabelLayout::remove_old_thing()
{
  if (count() == 2) {
    QLayoutItem* item = itemAt(1);
    if (QLayout* layout = item->layout(); layout != nullptr) {
      removeItem(layout);
      delete layout;  // NOLINT(cppcoreguidelines-owning-memory)
    } else if (QWidget* widget = item->widget(); widget != nullptr) {
      removeWidget(widget);
      delete widget;  // NOLINT(cppcoreguidelines-owning-memory)
    }
  }
}

}  // namespace omm
