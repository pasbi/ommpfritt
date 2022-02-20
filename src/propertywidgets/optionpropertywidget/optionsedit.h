#pragma once

#include "propertywidgets/multivalueedit.h"
#include <QComboBox>
#include <deque>

namespace omm
{
class NodeView;
class PrefixComboBox : public QComboBox
{
  Q_OBJECT
public:
  explicit PrefixComboBox(QWidget* parent = nullptr);
  QString prefix;
  bool eventFilter(QObject* o, QEvent* e) override;
  void showPopup() override;
  bool prevent_popup = false;

Q_SIGNALS:
  void popup_shown();
  void popup_hidden();

protected:
  void paintEvent(QPaintEvent*) override;
  QFrame* m_popup = nullptr;
};

class OptionsEdit
    : public PrefixComboBox
    , public MultiValueEdit<std::size_t>
{
public:
  explicit OptionsEdit(QWidget* parent = nullptr);
  void set_options(const std::deque<QString>& options);
  void set_value(const value_type& value) override;
  [[nodiscard]] value_type value() const override;
  void wheelEvent(QWheelEvent* event) override;

protected:
  void set_inconsistent_value() override;
};

}  // namespace omm
