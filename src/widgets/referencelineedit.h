#pragma once

#include <QComboBox>
#include "propertywidgets/multivalueedit.h"
#include "aspects/propertyowner.h"
#include "tags/tag.h"

class QMimeData;

namespace omm
{

class AbstractPropertyOwner;

class ReferenceLineEdit
  : public QComboBox
  , public MultiValueEdit<AbstractPropertyOwner*>
{
  Q_OBJECT
public:
  explicit ReferenceLineEdit(QWidget *parent = nullptr);
  ~ReferenceLineEdit() = default;
  void set_value(const value_type& value) override;
  value_type value() const override;
  void set_filter(const std::function<bool(const AbstractPropertyOwner*)>& filter);
  void set_null_label(const QString& value);
  void set_scene(Scene& scene);

protected:
  void set_inconsistent_value() override;
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent*) override;

private:
  bool can_drop(const QDropEvent& event) const;
  AbstractPropertyOwner* m_value;
  Scene* m_scene = nullptr;
  std::function<bool(const AbstractPropertyOwner*)> m_filter;
  std::vector<AbstractPropertyOwner*> m_possible_references;

  std::vector<omm::AbstractPropertyOwner*> collect_candidates();
  QString m_null_label;

public Q_SLOTS:
  void update_candidates();

private Q_SLOTS:
  void convert_text_to_placeholder_text();

Q_SIGNALS:
  void value_changed(value_type value);
};

}  // namespace omm
