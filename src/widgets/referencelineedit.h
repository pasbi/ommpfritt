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
  ~ReferenceLineEdit();
  void set_value(const value_type& value) override;
  value_type value() const override;
  void set_filter(AbstractPropertyOwner::Kind allowed_kinds);
  void set_filter(AbstractPropertyOwner::Flag required_flags);
  void set_null_label(const std::string& value);
  void set_scene(Scene& scene);

protected:
  void set_inconsistent_value() override;
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent*) override;

private:
  bool can_drop(const QMimeData& mime_data) const;
  AbstractPropertyOwner* m_value;
  Scene* m_scene = nullptr;
  AbstractPropertyOwner::Kind m_allowed_kinds = AbstractPropertyOwner::Kind::All;
  AbstractPropertyOwner::Flag m_required_flags = AbstractPropertyOwner::Flag::None;
  std::vector<AbstractPropertyOwner*> m_possible_references;

  std::vector<omm::AbstractPropertyOwner*> collect_candidates();
  std::string m_null_label;
  std::list<QMetaObject::Connection> m_connections;
public Q_SLOTS:
  void update_candidates();

private Q_SLOTS:
  void convert_text_to_placeholder_text();

Q_SIGNALS:
  void value_changed(value_type value);
};

}  // namespace omm
