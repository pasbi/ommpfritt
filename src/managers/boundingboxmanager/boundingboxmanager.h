#pragma once

#include "managers/manager.h"
#include "widgets/numericedit.h"
#include "aspects/propertyowner.h"

class QComboBox;

namespace omm
{

class AbstractPropertyOwner;
class AnchorWidget;

class BoundingBoxManager : public Manager, public AbstractPropertyObserver
{
  Q_OBJECT
public:
  BoundingBoxManager(Scene &scene);
  std::string type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "BoundingBoxManager");
  void on_property_value_changed(Property&, std::set<const void*> trace) override;

private:
  enum class Mode { Points = 0, Objects = 1 };
  DoubleNumericEdit *m_pos_x_field, *m_pos_y_field, *m_size_x_field, *m_size_y_field;
  QComboBox *m_mode_combo_box, *m_align_combo_box;
  AnchorWidget *m_anchor_widget;
  Mode current_mode() const;

private Q_SLOTS:
  void update_bounding_box();

};

}  // namespace omm
