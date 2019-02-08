#pragma once

#include <QTextEdit>
#include <QVBoxLayout>
#include <QLineEdit>
#include "observed.h"
#include "propertywidgets/multivalueedit.h"
#include "properties/stringproperty.h"
#include "widgets/filepathedit.h"
#include "widgets/codeedit.h"

namespace omm
{

class AbstractTextEditAdapter : public MultiValueEdit<std::string>
{
public:
  AbstractTextEditAdapter(const on_value_changed_t& on_value_changed);
  virtual void clear() = 0;
  virtual QWidget* as_widget() = 0;
};

template<typename TextEdit> class TextEditAdapter;

template<> class TextEditAdapter<QLineEdit>
  : public QLineEdit, public AbstractTextEditAdapter
{
public:
  using edit_type = QLineEdit;
  TextEditAdapter(const on_value_changed_t& on_value_changed, QWidget* parent = nullptr);
  void set_inconsistent_value() override;
  void set_value(const std::string& ph) override;
  void clear() override;
  std::string value() const override;
  QWidget* as_widget() override;
};

template<> class TextEditAdapter<QTextEdit>
  : public QTextEdit, public AbstractTextEditAdapter
{
public:
  using edit_type = QTextEdit;
  TextEditAdapter(const on_value_changed_t& on_value_changed, QWidget* parent = nullptr);
  void set_inconsistent_value() override;
  void set_value(const std::string& ph) override;
  void clear() override;
  std::string value() const override;
  QWidget* as_widget() override;
};

template<> class TextEditAdapter<FilePathEdit>
  : public FilePathEdit, public AbstractTextEditAdapter
{
public:
  using edit_type = FilePathEdit;
  TextEditAdapter(const on_value_changed_t& on_value_changed, QWidget* parent = nullptr);
  void set_inconsistent_value() override;
  void set_value(const std::string& ph) override;
  void clear() override;
  std::string value() const override;
  QWidget* as_widget() override;
};

template<> class TextEditAdapter<CodeEdit>
  : public CodeEdit, public AbstractTextEditAdapter
{
public:
  using edit_type = CodeEdit;
  using CodeEdit::CodeEdit;
  TextEditAdapter(const on_value_changed_t& on_value_changed, QWidget* parent = nullptr);
  void set_inconsistent_value() override;
  void set_value(const std::string& ph) override;
  void clear() override;
  std::string value() const override;
  QWidget* as_widget() override;
};


// template<typename TextEditAdapter>
// class GenericTextEdit : public QWidget
// {
// public:
//   using edit_type = typename TextEditAdapter::edit_type;
//   explicit GenericTextEdit(QWidget* parent, const on_value_changed_t& on_value_changed)
//     : AbstractGenericTextEdit(parent, on_value_changed)
//     , m_text_edit(std::make_unique<edit_type>(this).release())
//   {
//     setLayout(std::make_unique<QVBoxLayout>(this).release());
//     layout()->addWidget(m_text_edit);
//   }

//   void set_value(const std::string& text)
//   {
//     if (TextEditAdapter(*m_text_edit).text() != text) {
//       TextEditAdapter(*m_text_edit).set_text(text);
//     }
//   }

//   void set_placeholder_text(const std::string& text)
//   {
//     m_text_edit->setPlaceholderText(QString::fromStdString(text));
//   }

//   void set_inconsistent_value()
//   {
//     m_text_edit->setPlaceholderText(tr("<multiple values>"));
//     m_text_edit->clear();
//   }

// protected:
//   typename edit_type* const m_text_edit;
// };

// class MultiLineTextEdit : public GenericTextEdit<QTextEdit>
// {
// public:
//   explicit MultiLineTextEdit(QWidget* parent, const on_value_changed_t& on_value_changed);
//   std::string value() const override;
// };

// class SingleLineTextEdit : public GenericTextEdit<QLineEdit>
// {
// public:
//   explicit SingleLineTextEdit(QWidget* parent, const on_value_changed_t& on_value_changed);
//   std::string value() const override;
// };

// class FilePathTextEdit : public GenericTextEdit<FilePathEdit>
// {
// public:
//   explicit FilePathTextEdit(QWidget* parent, const on_value_changed_t& on_value_changed);
//   std::string value() const override;
// };

// class CodeTextEdit : public GenericTextEdit<CodeEdit>
// {
// public:
//   explicit CodeTextEdit(QWidget* parent, const on_value_changed_t& on_value_changed);
//   std::string value() const override;
// };

}  // namespace omm
