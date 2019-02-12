#include "propertywidgets/referencepropertywidget/referencepropertywidget.h"

#include "propertywidgets/multivalueedit.h"
#include "widgets/referencelineedit.h"
#include "properties/typedproperty.h"

namespace
{

template<typename PropertyT, typename ReturnT, typename F>
ReturnT get(const std::set<omm::Property*>& properties, const F& getter)
{
  const auto typed_properties = transform<const PropertyT*>(properties, [](const auto* property) {
    assert(std::holds_alternative<typename PropertyT::value_type>(property->variant_value()));
    return static_cast<const PropertyT*>(property);
  });
  const auto values = ::transform<ReturnT>(typed_properties, getter);
  assert(::is_uniform(values));
  return *values.begin();
}

}  // namespace

namespace omm
{

ReferencePropertyWidget
::ReferencePropertyWidget( Scene& scene, const std::string& key,
                           const std::set<Property*>& properties )
  : PropertyWidget(scene, key, properties)
{
  const auto on_value_changed = [this](const auto& reference) {
    set_properties_value(reference);
  };
  auto line_edit = std::make_unique<ReferenceLineEdit>(scene, on_value_changed);

  const auto get_allowed_kinds = [](const ReferenceProperty* p) { return p->allowed_kinds(); };
  const auto get_required_flags = [](const ReferenceProperty* p) { return p->required_flags(); };

  line_edit->set_filter(get<ReferenceProperty, AbstractPropertyOwner::Kind>( properties,
                                                                             get_allowed_kinds) );
  line_edit->set_filter(get<ReferenceProperty, AbstractPropertyOwner::Flag>( properties,
                                                                             get_required_flags) );

  m_line_edit = line_edit.get();
  set_default_layout(std::move(line_edit));

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
