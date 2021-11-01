#pragma once

#include "propertywidgets/multivalueedit.h"
#include "properties/propertyfilter.h"
#include <QComboBox>

namespace omm
{
class AbstractPropertyOwner;
class Scene;

class ReferenceLineEdit
    : public QComboBox
    , public MultiValueEdit<AbstractPropertyOwner*>
{
  Q_OBJECT
public:
  explicit ReferenceLineEdit(QWidget* parent = nullptr);
  void set_value(const value_type& value) override;
  [[nodiscard]] value_type value() const override;
  void set_filter(const PropertyFilter& filter);
  void set_null_label(const QString& value);
  void set_scene(Scene& scene);

protected:
  void set_inconsistent_value() override;
  void mouseDoubleClickEvent(QMouseEvent*) override;
  bool eventFilter(QObject* o, QEvent* e) override;

private:
  [[nodiscard]] bool can_drop(const QDropEvent& event) const;
  AbstractPropertyOwner* m_value = nullptr;
  Scene* m_scene = nullptr;
  PropertyFilter m_filter;
  std::vector<AbstractPropertyOwner*> m_possible_references;

  std::vector<omm::AbstractPropertyOwner*> collect_candidates();
  QString m_null_label;

  bool drag_enter(QDragEnterEvent& event);
  bool drop(QDropEvent& event);

public:
  void update_candidates();

private:
  void convert_text_to_placeholder_text();

Q_SIGNALS:
  void value_changed(omm::AbstractPropertyOwner*);
};

}  // namespace omm
