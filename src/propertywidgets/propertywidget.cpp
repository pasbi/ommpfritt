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
    property->Observed<AbstractPropertyObserver>::register_observer(*this);
  }
  connect(&m_update_timer, &QTimer::timeout, [this]() { update_edit(); });
  m_update_timer.setSingleShot(true);
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

void AbstractPropertyWidget::on_property_value_changed(Property&, std::set<const void *> trace)
{
  // wait until other properties have updated (important for MultiValueEdit)
  m_update_timer.start(0);
}

}  // namespace omm
