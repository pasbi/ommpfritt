#include "propertywidgets/referencepropertywidget.h"

#include "propertywidgets/multivalueedit.h"
#include "propertywidgets/referencelineedit.h"
#include "properties/typedproperty.h"

namespace
{

template<typename SetOfProperties>
omm::AbstractPropertyOwner::Kind get_allowed_kinds(SetOfProperties&& properties)
{
  const auto f = [](const auto property) { return property->allowed_kinds(); };
  const auto allowed_kindss = ::transform<omm::AbstractPropertyOwner::Kind>(properties, f);
  assert(::is_uniform(allowed_kindss));
  return *allowed_kindss.begin();
}

}  // namespace

namespace omm
{

ReferencePropertyWidget
::ReferencePropertyWidget(Scene& scene, const Property::SetOfProperties& properties)
  : PropertyWidget(scene, properties)
{
  auto line_edit = std::make_unique<ReferenceLineEdit>( scene,
                                                        get_allowed_kinds(this->properties()) );
  m_line_edit = line_edit.get();
  set_default_layout(std::move(line_edit));

  connect(m_line_edit, &ReferenceLineEdit::reference_changed, [this](const auto& reference) {
    set_properties_value(reference);
  });

  on_property_value_changed();
}

void ReferencePropertyWidget::on_property_value_changed()
{
  m_line_edit->blockSignals(true);
  m_line_edit->set_values(get_properties_values());
  m_line_edit->blockSignals(false);
}

std::string ReferencePropertyWidget::type() const
{
  return "ReferencePropertyWidget";
}

}  // namespace omm
