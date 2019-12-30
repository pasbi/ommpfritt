#include "propertywidgets/propertywidget.h"

#include <QBoxLayout>
#include <QLabel>
#include "widgets/animationbutton.h"

namespace omm
{

AbstractPropertyWidget::AbstractPropertyWidget(Scene& scene, const std::set<Property*>& properties)
  : scene(scene)
  , m_properties(properties)
{
  for (Property* property : properties) {
    connect(property, SIGNAL(value_changed(Property*)),
            this, SLOT(on_property_value_changed(Property*)));
    connect(property, SIGNAL(enabledness_changed(bool)),
            this, SLOT(update_enabledness()));
  }
  update_enabledness();
}

void AbstractPropertyWidget::set_default_layout(std::unique_ptr<QWidget> other)
{
  other->setParent(this);
  auto layout = std::make_unique<QHBoxLayout>();
  layout->addWidget(other.release(), 1);
  layout->setContentsMargins(0, 3, 0, 3);
  setLayout(layout.release());
}

void AbstractPropertyWidget::on_property_value_changed(Property*)
{
  // wait until other properties have updated (important for MultiValueEdit)
  QTimer::singleShot(1, this, SLOT(update_edit()));
}

void AbstractPropertyWidget::update_enabledness()
{
  const bool is_enabled = std::all_of(m_properties.begin(), m_properties.end(),
                                      [](const Property* property)
  {
    return property->is_enabled();
  });

  setEnabled(is_enabled);
}

}  // namespace omm
