#include "propertywidgets/propertywidget.h"

#include <QBoxLayout>
#include <QLabel>

#include "propertywidgets/numericpropertywidget.h"
#include "propertywidgets/stringpropertywidget.h"
#include "propertywidgets/transformationpropertywidget.h"

namespace
{

std::string get_label(const omm::AbstractPropertyWidget::SetOfProperties& properties)
{
  assert(properties.size() > 0);
  const auto label = (*properties.begin())->label();
#ifndef NDEBUG
  for (const auto& property : properties) {
    if (property->label() != label) {
      LOG(WARNING) << "Inconsistent label: '" << label << "' != '" << property->label() << "'.";
    }
  }
#endif
  return label;
}

}  // namespace

namespace omm
{

AbstractPropertyWidget::AbstractPropertyWidget(const SetOfProperties& properties)
  : m_label(get_label(properties))
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

void AbstractPropertyWidget::register_propertywidgets()
{
#define REGISTER_PROPERTYWIDGET(TYPE) AbstractPropertyWidget::register_type<TYPE>(#TYPE);

  REGISTER_PROPERTYWIDGET(IntegerPropertyWidget);
  REGISTER_PROPERTYWIDGET(FloatPropertyWidget);
  REGISTER_PROPERTYWIDGET(StringPropertyWidget);
  REGISTER_PROPERTYWIDGET(TransformationPropertyWidget);
  // REGISTER_PROPERTY(ReferenceProperty);

#undef REGISTER_PROPERTY
}

std::string AbstractPropertyWidget::label() const
{
  return m_label;
}

}  // namespace omm
