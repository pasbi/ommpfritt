#pragma once

#include <QTextEdit>
#include <QVBoxLayout>
#include <QLineEdit>
#include "propertywidgets/stringpropertywidget/fontcombobox.h"
#include "propertywidgets/multivalueedit.h"
#include "properties/stringproperty.h"
#include "widgets/filepathedit.h"
#include "widgets/codeedit.h"

namespace omm
{

class AbstractTextEditAdapter : public MultiValueEdit<QString>
{
public:
  virtual ~AbstractTextEditAdapter() = default;
  virtual void clear() = 0;
  virtual QWidget* as_widget() = 0;
};

template<typename TextEdit> class TextEditAdapter;

template<> class TextEditAdapter<QLineEdit>
  : public QLineEdit, public AbstractTextEditAdapter
{
public:
  using edit_type = QLineEdit;
  using QLineEdit::QLineEdit;
  void set_inconsistent_value() override;
  void set_value(const QString& ph) override;
  void clear() override;
  QString value() const override;
  QWidget* as_widget() override;
};

template<> class TextEditAdapter<QTextEdit>
  : public QTextEdit, public AbstractTextEditAdapter
{
public:
  using edit_type = QTextEdit;
  using QTextEdit::QTextEdit;
  explicit TextEditAdapter(QWidget* parent = nullptr);
  void set_inconsistent_value() override;
  void set_value(const QString& ph) override;
  void clear() override;
  QString value() const override;
  QWidget* as_widget() override;
};

template<> class TextEditAdapter<FilePathEdit>
  : public FilePathEdit, public AbstractTextEditAdapter
{
public:
  using edit_type = FilePathEdit;
  using FilePathEdit::FilePathEdit;
  void set_inconsistent_value() override;
  void set_value(const QString& ph) override;
  void clear() override;
  QString value() const override;
  QWidget* as_widget() override;
};

template<> class TextEditAdapter<CodeEdit>
  : public CodeEdit, public AbstractTextEditAdapter
{
public:
  using edit_type = CodeEdit;
  using CodeEdit::CodeEdit;
  void set_inconsistent_value() override;
  void set_value(const QString& ph) override;
  void clear() override;
  QString value() const override;
  QWidget* as_widget() override;
};

template<> class TextEditAdapter<FontComboBox>
  : public FontComboBox, public AbstractTextEditAdapter
{
public:
  using edit_type = FontComboBox;
  using FontComboBox::FontComboBox;
  void set_inconsistent_value() override;
  void set_value(const QString& ph) override;
  void clear() override;
  QString value() const override;
  QWidget* as_widget() override;
};

}  // namespace omm
