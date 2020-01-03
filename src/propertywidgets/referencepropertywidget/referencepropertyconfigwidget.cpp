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
  std::map<omm::Kind, QCheckBox*> map;
  map.insert( std::pair(omm::Kind::Tag,
              new QCheckBox(omm::ReferencePropertyConfigWidget::tr("Tag"))) );
  map.insert( std::pair(omm::Kind::Style,
              new QCheckBox(omm::ReferencePropertyConfigWidget::tr("Style"))) );
  map.insert( std::pair(omm::Kind::Object,
              new QCheckBox(omm::ReferencePropertyConfigWidget::tr("Object"))) );
  map.insert( std::pair(omm::Kind::Tool,
              new QCheckBox(omm::ReferencePropertyConfigWidget::tr("Tool"))) );
  map.insert( std::pair(omm::Kind::Node,
              new QCheckBox(omm::ReferencePropertyConfigWidget::tr("Node"))) );
  return map;
}

auto make_required_flags_checkboxes()
{
  std::map<omm::Flag, QCheckBox*> map;
  map.insert( std::pair(omm::Flag::Convertable,
              new QCheckBox(omm::ReferencePropertyConfigWidget::tr("convertable"))) );
  map.insert( std::pair(omm::Flag::HasScript,
              new QCheckBox(omm::ReferencePropertyConfigWidget::tr("has script"))) );
  map.insert( std::pair(omm::Flag::IsPathLike,
              new QCheckBox(omm::ReferencePropertyConfigWidget::tr("is path like"))) );
  map.insert( std::pair(omm::Flag::IsView,
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

  required_flags_layout->addWidget(new QLabel(tr("Requirements:")));
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
  using Filter = ReferenceProperty::Filter;
  const auto filter = configuration.get<Filter>(ReferenceProperty::FILTER_POINTER,
                                                Filter::accept_anything());
  for (auto&& [_, checkbox] : m_allowed_kind_checkboxes) {
    checkbox->setChecked(false);
  }
  for (const Literal<Kind>& literal : filter.kind.terms) {
    if (literal.value) {
      m_allowed_kind_checkboxes.at(literal)->setChecked(true);
    } else {
      LWARNING << "ReferencePropertyConfigWidet cannot handle negated literals";
    }
  }

  for (auto&& [_, checkbox] : m_required_flag_checkboxes) {
    checkbox->setChecked(false);
  }
  if (filter.flag.terms.size() != 1) {
    LWARNING << "Invalid number of disjunctions. "
                "ReferencePropertyConfigWidget can only handle a single disjunction.";
  } else {
    const auto& conjunction = *filter.flag.terms.begin();
    for (const auto& literal : conjunction.terms) {
      if (literal) {
        m_required_flag_checkboxes.at(literal)->setChecked(true);
      } else {
        LWARNING << "ReferencePropertyConfigWidet cannot handle negated literals";
      }
    }
  }
}

void ReferencePropertyConfigWidget::update(Property::Configuration &configuration) const
{
  Disjunction<Kind> kinds;
  DNF<Flag> flags;

  {
    Conjunction<Flag> conjunction;
    for (auto&& [ flag, checkbox ]: m_required_flag_checkboxes) {
      if (checkbox->isChecked()) {
        conjunction.terms.insert(flag);
      }
    }
    flags.terms.insert(conjunction);
  }

  for (auto&& [ kind, checkbox ]: m_allowed_kind_checkboxes) {
    if (checkbox->isChecked()) {
      kinds.terms.insert( kind );
    }
  }

  configuration[ReferenceProperty::FILTER_POINTER] = ReferenceProperty::Filter(kinds, flags);
}

}  // namespace omm
