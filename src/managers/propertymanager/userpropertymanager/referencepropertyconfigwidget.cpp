#include "managers/propertymanager/userpropertymanager/referencepropertyconfigwidget.h"

#include <QCheckBox>
#include <QLayout>

auto make_allowed_kinds_checkboxes(QWidget* parent)
{
  const auto make_cb = [parent](const std::string& label) {
    return std::make_unique<QCheckBox>(QString::fromStdString(label), parent).release();
  };
  std::map<omm::AbstractPropertyOwner::Kind, QCheckBox*> map;
  map.insert(std::pair(omm::AbstractPropertyOwner::Kind::Tag, make_cb("Tag")));
  map.insert(std::pair(omm::AbstractPropertyOwner::Kind::Style, make_cb("Style")));
  map.insert(std::pair(omm::AbstractPropertyOwner::Kind::Object, make_cb("Object")));
  return map;
}

namespace omm
{

ReferencePropertyConfigWidget::ReferencePropertyConfigWidget(QWidget* parent, Property& property)
  : PropertyConfigWidget(parent, property)
{
  using Kind = AbstractPropertyOwner::Kind;
  for (auto [kind, check_box] : make_allowed_kinds_checkboxes(parent)) {
    auto& reference_property = static_cast<ReferenceProperty&>(property);
    layout()->addWidget(check_box);
    check_box->setChecked(!!(reference_property.allowed_kinds() & kind));
    connect(check_box, &QCheckBox::clicked, [kind = kind, &reference_property](bool checked) {
      auto current = reference_property.allowed_kinds();
      reference_property.set_allowed_kinds(checked ? current | kind : current & ~kind);
    });
  }
}

std::string ReferencePropertyConfigWidget::type() const
{
  return TYPE;
}

}  // namespace omm
