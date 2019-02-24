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

}  // namespace omm
