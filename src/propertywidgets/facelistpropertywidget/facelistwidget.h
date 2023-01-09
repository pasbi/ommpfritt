#pragma once

#include "propertywidgets/multivalueedit.h"
#include "facelist.h"
#include <QWidget>

class QCheckBox;
class QListWidget;
class QPushButton;

namespace omm
{

class FaceListWidget : public QWidget, public MultiValueEdit<FaceList>
{
  Q_OBJECT
public:
  explicit FaceListWidget(QWidget* parent = nullptr);
  void set_value(const value_type& value) override;
  [[nodiscard]] value_type value() const override;

protected:
  void set_inconsistent_value() override;

private:
  QCheckBox* m_cb_invert;
  QListWidget* m_lw;
  QPushButton* m_pb_clear;
};

}  // namespace omm
