#include "propertywidgets/propertywidget.h"

#include <QBoxLayout>
#include <QLabel>

#include "propertywidgets/numericpropertywidget.h"
#include "propertywidgets/stringpropertywidget.h"
#include "propertywidgets/transformationpropertywidget.h"
#include "propertywidgets/referencepropertywidget.h"

namespace omm
{

AbstractPropertyWidget
::AbstractPropertyWidget(Scene& scene, const Property::SetOfProperties& properties)
  : m_label(Property::get_label(properties))
  , m_scene(scene)
{

}

AbstractPropertyWidget::~AbstractPropertyWidget()
{
}

void AbstractPropertyWidget::set_default_layout(std::unique_ptr<QWidget> other)
{
  auto layout = std::make_unique<QHBoxLayout>();
  layout->addWidget(make_label_widget().release(), 0);
  layout->addWidget(other.release(), 1);
  setLayout(layout.release());
}

std::unique_ptr<QWidget> AbstractPropertyWidget::make_label_widget() const
{
  return std::make_unique<QLabel>(QString::fromStdString(label()));
}

std::string AbstractPropertyWidget::label() const
{
  return m_label;
}

}  // namespace omm
