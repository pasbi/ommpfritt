#include "propertywidgets/propertywidget.h"

#include <QBoxLayout>
#include <QLabel>

#include "propertywidgets/integerpropertywidget.h"
#include "propertywidgets/stringpropertywidget.h"
#include "propertywidgets/transformationpropertywidget.h"

namespace omm
{

AbstractPropertyWidget::~AbstractPropertyWidget()
{
}

void AbstractPropertyWidget::set_default_layout(std::unique_ptr<QWidget> other)
{
  setLayout(new QHBoxLayout());
  layout()->addWidget(make_label_widget().release());
  layout()->addWidget(other.release());
}

std::unique_ptr<QWidget> AbstractPropertyWidget::make_label_widget() const
{
  return std::make_unique<QLabel>(QString::fromStdString(label()));
}

void AbstractPropertyWidget::register_propertywidgets()
{
#define REGISTER_PROPERTYWIDGET(TYPE) AbstractPropertyWidget::register_type<TYPE>(#TYPE);

  REGISTER_PROPERTYWIDGET(IntegerPropertyWidget);
  REGISTER_PROPERTYWIDGET(StringPropertyWidget);
  REGISTER_PROPERTYWIDGET(TransformationPropertyWidget);
  // REGISTER_PROPERTY(ReferenceProperty);

#undef REGISTER_PROPERTY
}

}  // namespace omm
