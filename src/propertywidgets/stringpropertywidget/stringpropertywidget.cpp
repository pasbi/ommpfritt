#include "propertywidgets/stringpropertywidget/stringpropertywidget.h"

#include "properties/typedproperty.h"
#include "propertywidgets/multivalueedit.h"
#include "propertywidgets/stringpropertywidget/texteditadapter.h"

#include <QLabel>

namespace omm
{
StringPropertyWidget ::StringPropertyWidget(Scene& scene, const std::set<Property*>& properties)
    : PropertyWidget(scene, properties)
{
  const auto mode = configuration<StringProperty::Mode>(StringProperty::MODE_PROPERTY_KEY);

  auto edit = [this, mode]() {
    switch (mode) {
    case StringProperty::Mode::MultiLine: {
      auto edit = std::make_unique<TextEditAdapter<QTextEdit>>();
      QObject::connect(edit.get(), &QTextEdit::textChanged, [edit = edit.get(), this]() {
        set_properties_value(edit->value());
      });
      return std::unique_ptr<AbstractTextEditAdapter>(edit.release());
    }
    case StringProperty::Mode::SingleLine: {
      auto edit = std::make_unique<TextEditAdapter<QLineEdit>>();
      QObject::connect(edit.get(), &QLineEdit::textChanged, [this](const QString& text) {
        set_properties_value(text);
      });
      return std::unique_ptr<AbstractTextEditAdapter>(edit.release());
    }
    case StringProperty::Mode::FilePath: {
      auto edit = std::make_unique<TextEditAdapter<FilePathEdit>>();
      QObject::connect(edit.get(), &FilePathEdit::path_changed, [this](const QString& text) {
        set_properties_value(text);
      });
      return std::unique_ptr<AbstractTextEditAdapter>(edit.release());
    }
    case StringProperty::Mode::Code: {
      auto edit = std::make_unique<TextEditAdapter<CodeEdit>>();
      QObject::connect(edit.get(), &CodeEdit::code_changed, [this](const QString& text) {
        set_properties_value(text);
      });
      return std::unique_ptr<AbstractTextEditAdapter>(edit.release());
    }
    case StringProperty::Mode::Font: {
      auto edit = std::make_unique<TextEditAdapter<FontComboBox>>();
      QObject::connect(edit.get(), &QFontComboBox::currentTextChanged, [this](const QString& text) {
        set_properties_value(text);
      });
      return std::unique_ptr<AbstractTextEditAdapter>(edit.release());
    }
    default:
      Q_UNREACHABLE();
    }
  }();

  m_text_edit = edit.get();
  //  connect(m_text_edit, &AbstractTextEditAdapter::value_changed, [this](QString text) {
  //    this->set_properties_value(text);
  //  });

  auto text_edit_widget = std::unique_ptr<QWidget>(edit.release()->as_widget());
  auto vlayout = std::make_unique<LabelLayout>();
  vlayout->set_label(label());
  vlayout->set_thing(std::move(text_edit_widget));
  setLayout(vlayout.release());

  StringPropertyWidget::update_edit();
}

void StringPropertyWidget::update_edit()
{
  QSignalBlocker blocker(m_text_edit->as_widget());
  m_text_edit->set_values(get_properties_values());
}

void StringPropertyWidget::update_configuration()
{
  // TODO
}

}  // namespace omm
