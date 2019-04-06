#include "propertywidgets/propertywidget.h"

#include <QBoxLayout>
#include <QLabel>
#include <QTimer>

namespace omm
{

AbstractPropertyWidget::AbstractPropertyWidget(Scene& scene, const std::set<Property*>& properties)
  : scene(scene)
  , m_properties(properties)
{
  for (auto&& property : properties) {
    property->Observed<AbstractPropertyObserver>::register_observer(*this);
  }
}

AbstractPropertyWidget::~AbstractPropertyWidget()
{
  for (auto&& property : m_properties) {
    property->Observed<AbstractPropertyObserver>::unregister_observer(*this);
  }
}

void AbstractPropertyWidget::set_default_layout(std::unique_ptr<QWidget> other)
{
  auto layout = std::make_unique<QHBoxLayout>();
  layout->addWidget(other.release(), 1);
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout.release());
}

void AbstractPropertyWidget::on_property_value_changed(Property&)
{
  // wait until other properties have updated (important for MultiValueEdit)
  QTimer::singleShot(1, [this]() { update_edit(); });
}

}  // namespace omm
