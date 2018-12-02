#include "propertywidgets/boolpropertywidget.h"

#include <QCheckBox>

#include "propertywidgets/multivalueedit.h"
#include "properties/typedproperty.h"


namespace omm
{

class BoolPropertyWidget::CheckBox : public QCheckBox, public MultiValueEdit<bool>
{
public:
  explicit CheckBox();
  void set_value(const value_type& value) override;
  value_type value() const override;
  void nextCheckState() override;

protected:
  void set_inconsistent_value() override;
};

BoolPropertyWidget::CheckBox::CheckBox()
{
  setTristate(true);
}

void BoolPropertyWidget::CheckBox::set_value(const value_type& value)
{
  setChecked(value);
}

void BoolPropertyWidget::CheckBox::set_inconsistent_value()
{
  setCheckState(Qt::PartiallyChecked);
}

BoolPropertyWidget::CheckBox::value_type BoolPropertyWidget::CheckBox::value() const
{
  return isChecked();
}

void BoolPropertyWidget::CheckBox::nextCheckState()
{
  setChecked(!isChecked());
}


BoolPropertyWidget::BoolPropertyWidget(Scene& scene, const Property::SetOfProperties& properties)
  : PropertyWidget(scene, properties)
{
  auto checkbox = std::make_unique<CheckBox>();
  m_checkbox = checkbox.get();
  set_default_layout(std::move(checkbox));

  connect(m_checkbox, &QCheckBox::stateChanged, [this](const int state) {
    set_properties_value(state == Qt::Checked);
  });

  on_property_value_changed();
}

void BoolPropertyWidget::on_property_value_changed()
{
  m_checkbox->blockSignals(true);
  m_checkbox->set_values(get_properties_values());
  m_checkbox->blockSignals(false);
}

std::string BoolPropertyWidget::type() const
{
  return "BoolPropertyWidget";
}

}  // namespace omm
