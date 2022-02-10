#include "propertywidgets/referencepropertywidget/referencepropertyconfigwidget.h"
#include "enumnames.h"
#include "properties/referenceproperty.h"

#include <QCheckBox>
#include <QLabel>
#include <QLayout>

namespace
{
template<typename MapT> auto keys(const MapT& map)
{
  return util::transform<std::set>(map, [](const auto& kv) { return kv.first; });
}

template<typename E> auto make_checkboxes()
{
  std::map<E, QCheckBox*> map;
  for (E kind : omm::enumerate_enum<E>()) {
    map.insert({kind, new QCheckBox(omm::enum_name(kind, true))});
  }
  return map;
}

}  // namespace

namespace omm
{
ReferencePropertyConfigWidget::ReferencePropertyConfigWidget()
    : m_allowed_kind_checkboxes(make_checkboxes<Kind>()),
      m_required_flag_checkboxes(make_checkboxes<Flag>())
{
  auto layout = std::make_unique<QHBoxLayout>();
  auto allowed_kind_layout = std::make_unique<QVBoxLayout>();
  auto required_flags_layout = std::make_unique<QVBoxLayout>();

  for (auto [_, checkbox] : m_allowed_kind_checkboxes) {
    allowed_kind_layout->addWidget(checkbox);
  }
  allowed_kind_layout->addStretch();
  auto label = std::make_unique<QLabel>(tr("Requirements:"));
  required_flags_layout->addWidget(label.release());
  for (auto [_, checkbox] : m_required_flag_checkboxes) {
    required_flags_layout->addWidget(checkbox);
  }
  required_flags_layout->addStretch();

  layout->addLayout(allowed_kind_layout.release());
  layout->addLayout(required_flags_layout.release());
  setLayout(layout.release());
}

void ReferencePropertyConfigWidget::init(const PropertyConfiguration& configuration)
{
  using Filter = PropertyFilter;
  const auto filter = configuration.get<PropertyFilter>(ReferenceProperty::FILTER_POINTER,
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
      if (literal.value) {
        m_required_flag_checkboxes.at(literal)->setChecked(true);
      } else {
        LWARNING << "ReferencePropertyConfigWidet cannot handle negated literals";
      }
    }
  }
}

void ReferencePropertyConfigWidget::update(PropertyConfiguration& configuration) const
{
  Disjunction<Kind> kinds;
  DNF<Flag> flags;

  {
    Conjunction<Flag> conjunction;
    for (auto&& [flag, checkbox] : m_required_flag_checkboxes) {
      if (checkbox->isChecked()) {
        conjunction.terms.insert(flag);
      }
    }
    flags.terms.insert(conjunction);
  }

  for (auto&& [kind, checkbox] : m_allowed_kind_checkboxes) {
    if (checkbox->isChecked()) {
      kinds.terms.insert(kind);
    }
  }

  configuration.set(ReferenceProperty::FILTER_POINTER, PropertyFilter(kinds, flags));
}

}  // namespace omm
