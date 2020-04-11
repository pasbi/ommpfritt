#include "propertywidgets/referencepropertywidget/referencepropertywidget.h"

#include "propertywidgets/multivalueedit.h"
#include "widgets/referencelineedit.h"
#include "properties/typedproperty.h"

#include <QLabel>
#include <QVBoxLayout>

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
::ReferencePropertyWidget(Scene& scene, const std::set<Property*>& properties)
  : PropertyWidget(scene, properties)
{
  auto line_edit = std::make_unique<ReferenceLineEdit>();
  line_edit->set_scene(scene);
  connect(line_edit.get(), &ReferenceLineEdit::value_changed, [this](AbstractPropertyOwner* o) {
    set_properties_value(o);
  });

  const auto filter = get<ReferenceProperty, ReferenceProperty::Filter>(properties, [](const ReferenceProperty* p) {
    return p->filter();
  });

  line_edit->set_filter(filter);

  m_line_edit = line_edit.get();
  auto vlayout = std::make_unique<LabelLayout>();
  vlayout->set_label(label());
  vlayout->set_thing(std::make_unique<QLabel>());
  set_widget(std::move(line_edit));

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

}  // namespace omm
