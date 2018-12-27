#include "propertywidgets/referencepropertywidget/referencepropertywidget.h"

#include "propertywidgets/multivalueedit.h"
#include "propertywidgets/referencepropertywidget/referencelineedit.h"
#include "properties/typedproperty.h"

namespace
{

omm::AbstractPropertyOwner::Kind get_allowed_kinds(const std::set<omm::Property*>& properties)
{
  const auto f = [](const auto* property) {
    using property_type = omm::ReferencePropertyWidget::property_type;
    assert(std::holds_alternative<property_type::value_type>(property->variant_value()));
    return static_cast<const property_type&>(*property).allowed_kinds();
  };
  const auto allowed_kindss = ::transform<omm::AbstractPropertyOwner::Kind>(properties, f);
  assert(::is_uniform(allowed_kindss));
  return *allowed_kindss.begin();
}

}  // namespace

namespace omm
{

ReferencePropertyWidget
::ReferencePropertyWidget(Scene& scene, const std::set<Property*>& properties)
  : PropertyWidget(scene, properties)
{
  auto line_edit = std::make_unique<ReferenceLineEdit>( scene,
                                                        get_allowed_kinds(this->properties()) );
  m_line_edit = line_edit.get();
  set_default_layout(std::move(line_edit));

  connect(m_line_edit, &ReferenceLineEdit::reference_changed, [this](const auto& reference) {
    set_properties_value(reference);
  });

  update_edit();
}

ReferencePropertyWidget::~ReferencePropertyWidget()
{
}

void ReferencePropertyWidget::update_edit()
{
  QSignalBlocker blocker(m_line_edit);
  m_line_edit->set_values(get_properties_values());
}

std::string ReferencePropertyWidget::type() const
{
  return TYPE;
}

}  // namespace omm
