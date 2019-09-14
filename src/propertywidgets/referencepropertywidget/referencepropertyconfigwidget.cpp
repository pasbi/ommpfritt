#include "propertywidgets/referencepropertywidget/referencepropertyconfigwidget.h"

#include <QCheckBox>
#include <QLayout>
#include <QLabel>

namespace
{

template<typename MapT> auto keys(const MapT& map)
{
  return ::transform<typename MapT::value_type::second_type, std::set>(map, [](const auto& kv) {
    return kv.first;
  });
}

auto make_allowed_kinds_checkboxes()
{
  std::map<omm::AbstractPropertyOwner::Kind, QCheckBox*> map;
  map.insert( std::pair(omm::AbstractPropertyOwner::Kind::Tag,
              new QCheckBox(omm::ReferencePropertyConfigWidget::tr("Tag"))) );
  map.insert( std::pair(omm::AbstractPropertyOwner::Kind::Style,
              new QCheckBox(omm::ReferencePropertyConfigWidget::tr("Style"))) );
  map.insert( std::pair(omm::AbstractPropertyOwner::Kind::Object,
              new QCheckBox(omm::ReferencePropertyConfigWidget::tr("Object"))) );
  map.insert( std::pair(omm::AbstractPropertyOwner::Kind::Tool,
              new QCheckBox(omm::ReferencePropertyConfigWidget::tr("Tool"))) );
  return map;
}

auto make_required_flags_checkboxes()
{
  std::map<omm::AbstractPropertyOwner::Flag, QCheckBox*> map;
  map.insert( std::pair(omm::AbstractPropertyOwner::Flag::Convertable,
              new QCheckBox(omm::ReferencePropertyConfigWidget::tr("convertable"))) );
  map.insert( std::pair(omm::AbstractPropertyOwner::Flag::HasScript,
              new QCheckBox(omm::ReferencePropertyConfigWidget::tr("has script"))) );
  map.insert( std::pair(omm::AbstractPropertyOwner::Flag::IsPathLike,
              new QCheckBox(omm::ReferencePropertyConfigWidget::tr("is path like"))) );
  map.insert( std::pair(omm::AbstractPropertyOwner::Flag::IsView,
              new QCheckBox(omm::ReferencePropertyConfigWidget::tr("is view"))) );
  return map;
}

}  // namespace

namespace omm
{

ReferencePropertyConfigWidget::ReferencePropertyConfigWidget()
  : m_allowed_kind_checkboxes(make_allowed_kinds_checkboxes())
  , m_required_flag_checkboxes(make_required_flags_checkboxes())
{
  auto layout = std::make_unique<QHBoxLayout>();
  auto allowed_kind_layout = std::make_unique<QVBoxLayout>();
  auto required_flags_layout = std::make_unique<QVBoxLayout>();

  for (auto [_, checkbox] : m_allowed_kind_checkboxes) {
    allowed_kind_layout->addWidget(checkbox);
  }
  allowed_kind_layout->addStretch();

  for (auto [_, checkbox] : m_required_flag_checkboxes) {
    required_flags_layout->addWidget(checkbox);
  }
  required_flags_layout->addStretch();

  layout->addLayout(allowed_kind_layout.release());
  layout->addLayout(required_flags_layout.release());
  setLayout(layout.release());
}

void ReferencePropertyConfigWidget::init(const Property::Configuration &configuration)
{
  for (auto&& [ flag, checkbox ]: m_required_flag_checkboxes) {
    checkbox->setChecked(configuration.get<bool>(ReferenceProperty::FLAG_KEYS.at(flag), false));
  }
  for (auto&& [ kind, checkbox ]: m_allowed_kind_checkboxes) {
    checkbox->setChecked(configuration.get<bool>(ReferenceProperty::KIND_KEYS.at(kind), true));
  }
}

void ReferencePropertyConfigWidget::update(Property::Configuration &configuration) const
{
  for (auto&& [ flag, checkbox ]: m_required_flag_checkboxes) {
    configuration[ReferenceProperty::FLAG_KEYS.at(flag)] = checkbox->isChecked();
  }
  for (auto&& [ kind, checkbox ]: m_allowed_kind_checkboxes) {
    configuration[ReferenceProperty::KIND_KEYS.at(kind)] = checkbox->isChecked();
  }
}

}  // namespace omm
