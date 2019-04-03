#include "propertywidgets/optionspropertywidget/optionsedit.h"
#include "common.h"

namespace omm
{

OptionsEdit::OptionsEdit(const on_value_changed_t& on_value_changed)
  : MultiValueEdit<size_t>(on_value_changed)
{
  const auto s = static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged);
  connect(this, s, [f=on_value_changed, this](int index) { f(index); });
}

void OptionsEdit::set_value(const value_type& value) { setCurrentIndex(value); }
void OptionsEdit::set_inconsistent_value() { setCurrentIndex(-1); }
OptionsEdit::value_type OptionsEdit::value() const { return currentIndex(); }
void OptionsEdit::set_options(const std::vector<std::string>& options)
{
  clear();
  addItems(::transform<QString, QList>(options, [](const std::string& string) {
    return QString::fromStdString(string);
  }));
}

}  // namespace omm
