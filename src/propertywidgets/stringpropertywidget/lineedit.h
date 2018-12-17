#pragma once

#include <QWidget>
#include "propertywidgets/multivalueedit.h"
#include "properties/stringproperty.h"

class QTextEdit;
class QLineEdit;

namespace omm
{

class LineEdit : public QWidget, public MultiValueEdit<std::string>
{

    Q_OBJECT
public:
  explicit LineEdit(StringProperty::LineMode mode);
  void set_value(const value_type& value) override;
  value_type value() const override;

protected:
  void set_inconsistent_value() override;

Q_SIGNALS:
  void textChanged();

private:
  QLineEdit* m_line_edit = nullptr;
  QTextEdit* m_text_edit = nullptr;
};

}  // namespace omm
