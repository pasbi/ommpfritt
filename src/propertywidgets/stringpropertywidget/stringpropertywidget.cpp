#include "propertywidgets/stringpropertywidget/stringpropertywidget.h"

#include "propertywidgets/multivalueedit.h"
#include "properties/typedproperty.h"
#include "propertywidgets/stringpropertywidget/texteditadapter.h"

namespace omm
{

StringPropertyWidget
::StringPropertyWidget(Scene& scene, const std::set<Property*>& properties)
  : PropertyWidget(scene, properties)
{
  const auto getter = std::mem_fn(&StringProperty::mode);
  const auto mode = Property::get_value<StringProperty::Mode, StringProperty>(properties, getter);

  auto edit = [this, mode]() {
    switch (mode) {
    case StringProperty::Mode::MultiLine: {
      auto edit = std::make_unique<TextEditAdapter<QTextEdit>>(this);
      QObject::connect(edit.get(), &QTextEdit::textChanged, [edit=edit.get(), this]() {
        set_properties_value(edit->value());
      });
      return std::unique_ptr<AbstractTextEditAdapter>(edit.release());
    }
    case StringProperty::Mode::SingleLine: {
      auto edit = std::make_unique<TextEditAdapter<QLineEdit>>(this);
      QObject::connect(edit.get(), &QLineEdit::textChanged, [this](const QString& text) {
        set_properties_value(text.toStdString());
      });
      return std::unique_ptr<AbstractTextEditAdapter>(edit.release());
    }
    case StringProperty::Mode::FilePath: {
      auto edit = std::make_unique<TextEditAdapter<FilePathEdit>>(this);
      QObject::connect(edit.get(), &FilePathEdit::path_changed, [this](const std::string& text) {
        set_properties_value(text);
      });
      return std::unique_ptr<AbstractTextEditAdapter>(edit.release());
    }
    case StringProperty::Mode::Code: {
      auto edit = std::make_unique<TextEditAdapter<CodeEdit>>(this);
      QObject::connect(edit.get(), &CodeEdit::code_changed, [this](const std::string& text) {
        set_properties_value(text);
      });
      return std::unique_ptr<AbstractTextEditAdapter>(edit.release());
    }
    case StringProperty::Mode::Font: {
      auto edit = std::make_unique<TextEditAdapter<QFontComboBox>>(this);
      QObject::connect(edit.get(), &QFontComboBox::currentTextChanged, [this](const QString& text) {
        set_properties_value(text.toStdString());
      });
      return std::unique_ptr<AbstractTextEditAdapter>(edit.release());
    }
    default:
      Q_UNREACHABLE();
    }
  }();

  m_text_edit = edit.get();
//  connect(m_text_edit, &AbstractTextEditAdapter::value_changed, [this](QString text) {
//    this->set_properties_value(text.toStdString());
//  });

  auto text_edit_widget = std::unique_ptr<QWidget>(edit.release()->as_widget());
  set_default_layout(std::move(text_edit_widget));

  update_edit();
}

void StringPropertyWidget::update_edit()
{
  QSignalBlocker blocker(m_text_edit->as_widget());
  m_text_edit->set_values(get_properties_values());
}

std::string StringPropertyWidget::type() const
{
  return TYPE;
}

}  // namespace omm
