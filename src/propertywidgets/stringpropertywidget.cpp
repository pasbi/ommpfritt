#include "propertywidgets/stringpropertywidget.h"

#include <QLineEdit>

#include "propertywidgets/multivalueedit.h"
#include "properties/typedproperty.h"


namespace omm
{

class StringPropertyWidget::LineEdit : public QLineEdit, public MultiValueEdit<std::string>
{
public:
  explicit LineEdit();
  void set_value(const value_type& value) override;
  void set_inconsistent_value() override;
  value_type value() const override;
};

StringPropertyWidget::LineEdit::LineEdit()
{
  connect(this, &QLineEdit::textChanged, [this]() { setPlaceholderText(""); });
}

void StringPropertyWidget::LineEdit::set_value(const value_type& value)
{
  setText(QString::fromStdString(value));
}

void StringPropertyWidget::LineEdit::set_inconsistent_value()
{
  setPlaceholderText(tr("<multiple values>"));
  clear();
}

StringPropertyWidget::LineEdit::value_type StringPropertyWidget::LineEdit::value() const
{
  return text().toStdString();
}


StringPropertyWidget::StringPropertyWidget(const SetOfProperties& properties)
  : PropertyWidget(properties)
{
  auto line_edit = std::make_unique<LineEdit>();
  m_line_edit = line_edit.get();
  set_default_layout(std::move(line_edit));

  connect(m_line_edit, &QLineEdit::textChanged, [this](const QString& text) {
    for (auto property : m_properties) {
      property->set_value(text.toStdString());
    }
  });

  on_value_changed();
}

void StringPropertyWidget::on_value_changed()
{
  m_line_edit->set_values(
    ::transform<std::string>(m_properties, [](auto property) { return property->value(); })
  );
}

std::string StringPropertyWidget::type() const
{
  return "StringPropertyWidget";
}

}  // namespace omm
