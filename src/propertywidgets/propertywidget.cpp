#include "propertywidgets/propertywidget.h"

#include <QBoxLayout>
#include <QLabel>

namespace omm
{

AbstractPropertyWidget::AbstractPropertyWidget(Scene& scene, const std::set<Property*>& properties)
  : scene(scene)
  , m_properties(properties)
{
  for (auto&& property : properties) {
    connect(property, SIGNAL(value_changed(Property*)),
            this, SLOT(on_property_value_changed(Property*)));
  }
}

void AbstractPropertyWidget::set_default_layout(std::unique_ptr<QWidget> other)
{
  auto layout = std::make_unique<QHBoxLayout>();
  layout->addWidget(other.release(), 1);
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout.release());
}

void AbstractPropertyWidget::on_property_value_changed(Property*)
{
  // wait until other properties have updated (important for MultiValueEdit)
  QTimer::singleShot(1, this, SLOT(update_edit()));
}

}  // namespace omm
