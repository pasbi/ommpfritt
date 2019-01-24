#pragma once

#include <QTextEdit>
#include <QVBoxLayout>
#include <QLineEdit>
#include "observed.h"
#include "propertywidgets/multivalueedit.h"
#include "properties/stringproperty.h"
#include "widgets/filepathedit.h"

namespace omm
{

class AbstractGenericTextEdit : public QWidget, public MultiValueEdit<std::string>
{
public:
  explicit AbstractGenericTextEdit(QWidget* parent, const on_value_changed_t& on_value_changed);
  virtual void set_value(const std::string& text) = 0;
  virtual std::string value() const = 0;
  virtual void set_placeholder_text(const std::string& text) = 0;
};

template<typename TextEdit>
class GenericTextEdit : public AbstractGenericTextEdit
{
public:
  explicit GenericTextEdit(QWidget* parent, const on_value_changed_t& on_value_changed)
    : AbstractGenericTextEdit(parent, on_value_changed)
    , m_text_edit(std::make_unique<TextEdit>(this).release())
  {
    setLayout(std::make_unique<QVBoxLayout>(this).release());
    layout()->addWidget(m_text_edit);
  }

  void set_value(const std::string& text) override
  {
    if (value() != text) {
      m_text_edit->setText(QString::fromStdString(text));
    } else {
      // each set destroys the cursor position.
    }
  }

  void set_placeholder_text(const std::string& text) override
  {
    m_text_edit->setPlaceholderText(QString::fromStdString(text));
  }

  void set_inconsistent_value() override
  {
    m_text_edit->setPlaceholderText(tr("<multiple values>"));
    m_text_edit->clear();
  }

protected:
  TextEdit* const m_text_edit;
};

class MultiLineTextEdit : public GenericTextEdit<QTextEdit>
{
public:
  explicit MultiLineTextEdit(QWidget* parent, const on_value_changed_t& on_value_changed);
  std::string value() const override;
};

class SingleLineTextEdit : public GenericTextEdit<QLineEdit>
{
public:
  explicit SingleLineTextEdit(QWidget* parent, const on_value_changed_t& on_value_changed);
  std::string value() const override;
};

class FilePathTextEdit : public GenericTextEdit<FilePathEdit>
{
public:
  explicit FilePathTextEdit(QWidget* parent, const on_value_changed_t& on_value_changed);
  std::string value() const override;
};

}  // namespace omm
