#pragma once

#include "properties/stringproperty.h"
#include "propertywidgets/multivalueedit.h"
#include "propertywidgets/stringpropertywidget/fontcombobox.h"
#include "widgets/codeedit.h"
#include "widgets/filepathedit.h"
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>

namespace omm
{
class AbstractTextEditAdapter : public MultiValueEdit<QString>
{
public:
  virtual ~AbstractTextEditAdapter() = default;
  explicit AbstractTextEditAdapter() = default;
  AbstractTextEditAdapter(const AbstractTextEditAdapter&) = delete;
  AbstractTextEditAdapter(AbstractTextEditAdapter&&) = delete;
  AbstractTextEditAdapter& operator=(const AbstractTextEditAdapter&) = delete;
  AbstractTextEditAdapter& operator=(AbstractTextEditAdapter&&) = delete;
  virtual void clear() = 0;
  virtual QWidget* as_widget() = 0;
};

template<typename TextEdit> class TextEditAdapter;

template<>
class TextEditAdapter<QLineEdit>
    : public QLineEdit
    , public AbstractTextEditAdapter
{
public:
  using edit_type = QLineEdit;
  using QLineEdit::QLineEdit;
  void set_inconsistent_value() override;
  void set_value(const QString& text) override;
  void clear() override;
  [[nodiscard]] QString value() const override;
  [[nodiscard]] QWidget* as_widget() override;
};

template<>
class TextEditAdapter<QTextEdit>
    : public QTextEdit
    , public AbstractTextEditAdapter
{
public:
  using edit_type = QTextEdit;
  using QTextEdit::QTextEdit;
  explicit TextEditAdapter(QWidget* parent = nullptr);
  void set_inconsistent_value() override;
  void set_value(const QString& text) override;
  void clear() override;
  [[nodiscard]] QString value() const override;
  [[nodiscard]] QWidget* as_widget() override;
};

template<>
class TextEditAdapter<FilePathEdit>
    : public FilePathEdit
    , public AbstractTextEditAdapter
{
public:
  using edit_type = FilePathEdit;
  using FilePathEdit::FilePathEdit;
  void set_inconsistent_value() override;
  void set_value(const QString& text) override;
  void clear() override;
  [[nodiscard]] QString value() const override;
  [[nodiscard]] QWidget* as_widget() override;
};

template<>
class TextEditAdapter<CodeEdit>
    : public CodeEdit
    , public AbstractTextEditAdapter
{
public:
  using edit_type = CodeEdit;
  using CodeEdit::CodeEdit;
  void set_inconsistent_value() override;
  void set_value(const QString& text) override;
  void clear() override;
  [[nodiscard]] QString value() const override;
  [[nodiscard]] QWidget* as_widget() override;
};

template<>
class TextEditAdapter<FontComboBox>
    : public FontComboBox
    , public AbstractTextEditAdapter
{
public:
  using edit_type = FontComboBox;
  using FontComboBox::FontComboBox;
  void set_inconsistent_value() override;
  void set_value(const QString& text) override;
  void clear() override;
  [[nodiscard]] QString value() const override;
  [[nodiscard]] QWidget* as_widget() override;
};

}  // namespace omm
