#include "propertywidgets/referencepropertywidget/referencepropertyconfigwidget.h"

#include <QCheckBox>
#include <QLayout>
#include <QLabel>

auto make_allowed_kinds_checkboxes(QWidget* parent)
{
  const auto make_cb = [parent](const std::string& label) {
    return std::make_unique<QCheckBox>(QString::fromStdString(label), parent).release();
  };
  std::map<omm::AbstractPropertyOwner::Kind, QCheckBox*> map;
  map.insert( std::pair(omm::AbstractPropertyOwner::Kind::Tag,
              make_cb(omm::ReferencePropertyConfigWidget::tr("Tag").toStdString())) );
  map.insert( std::pair(omm::AbstractPropertyOwner::Kind::Style,
              make_cb(omm::ReferencePropertyConfigWidget::tr("Style").toStdString())) );
  map.insert( std::pair(omm::AbstractPropertyOwner::Kind::Object,
              make_cb(omm::ReferencePropertyConfigWidget::tr("Object").toStdString())) );
  return map;
}

auto make_required_flags_checkboxes(QWidget* parent)
{
  const auto make_cb = [parent](const std::string& label) {
    return std::make_unique<QCheckBox>(QString::fromStdString(label), parent).release();
  };
  std::map<omm::AbstractPropertyOwner::Flag, QCheckBox*> map;
  map.insert( std::pair(omm::AbstractPropertyOwner::Flag::Convertable,
              make_cb(omm::ReferencePropertyConfigWidget::tr("convertable").toStdString())) );
  map.insert( std::pair(omm::AbstractPropertyOwner::Flag::HasScript,
              make_cb(omm::ReferencePropertyConfigWidget::tr("has script").toStdString())) );
  map.insert( std::pair(omm::AbstractPropertyOwner::Flag::IsPathLike,
              make_cb(omm::ReferencePropertyConfigWidget::tr("is path like").toStdString())) );
  return map;
}

namespace omm
{

ReferencePropertyConfigWidget::ReferencePropertyConfigWidget(QWidget* parent, Property& property)
  : PropertyConfigWidget(parent, property)
{
  auto& reference_property = type_cast<ReferenceProperty&>(property);

  auto left_column = std::make_unique<QVBoxLayout>();
  left_column->addWidget(std::make_unique<QLabel>(tr("allowed:")).release());
  for (auto [kind, check_box] : make_allowed_kinds_checkboxes(parent)) {
    left_column->addWidget(check_box);
    check_box->setChecked(!!(reference_property.allowed_kinds() & kind));
    connect(check_box, &QCheckBox::clicked, [kind = kind, &reference_property](bool checked) {
      auto current = reference_property.allowed_kinds();
      reference_property.set_allowed_kinds(checked ? current | kind : current & ~kind);
    });
  }

  auto right_column = std::make_unique<QVBoxLayout>();
  right_column->addWidget(std::make_unique<QLabel>(tr("required:")).release());
  for (auto [flag, check_box] : make_required_flags_checkboxes(parent)) {
    right_column->addWidget(check_box);
    check_box->setChecked(!!(reference_property.required_flags() & flag));
    connect(check_box, &QCheckBox::clicked, [flag = flag, &reference_property](bool checked) {
      auto current = reference_property.required_flags();
      reference_property.set_required_flags(checked ? current | flag : current & ~flag);
    });
  }

  auto two_column_layout = std::make_unique<QHBoxLayout>();
  two_column_layout->addLayout(left_column.release());
  two_column_layout->addLayout(right_column.release());
  box_layout()->addLayout(two_column_layout.release());
  box_layout()->addStretch();
}

std::string ReferencePropertyConfigWidget::type() const
{
  return TYPE;
}

}  // namespace omm
