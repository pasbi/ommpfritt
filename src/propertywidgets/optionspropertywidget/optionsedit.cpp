#include "propertywidgets/optionspropertywidget/optionsedit.h"
#include "common.h"

namespace omm
{

void OptionsEdit::set_value(const value_type& value) { setCurrentIndex(value); }
void OptionsEdit::set_inconsistent_value() { setCurrentIndex(-1); }
OptionsEdit::value_type OptionsEdit::value() const { return currentIndex(); }
void OptionsEdit::set_options(const std::vector<QString>& options)
{
  clear();
  addItems(::transform<QString, QList>(options, ::identity));
}

}  // namespace omm
