#pragma once

#include <QComboBox>
#include "propertywidgets/multivalueedit.h"
#include "aspects/propertyowner.h"
#include "tags/tag.h"
#include "scene/abstractstructureobserver.h"

class QMimeData;

namespace omm
{

class AbstractPropertyOwner;

class ReferenceLineEdit
  : public QComboBox
  , public MultiValueEdit<AbstractPropertyOwner*>
  , public AbstractSimpleStructureObserver
{
public:
  explicit ReferenceLineEdit(Scene& scene, const on_value_changed_t& on_value_changed);
  ~ReferenceLineEdit();
  void set_value(const value_type& value) override;
  value_type value() const override;
  void structure_has_changed() override;
  void set_filter(AbstractPropertyOwner::Kind allowed_kinds);
  void set_filter(AbstractPropertyOwner::Flag required_flags);

protected:
  void set_inconsistent_value() override;
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
  bool can_drop(const QMimeData& mime_data) const;
  AbstractPropertyOwner* m_value;
  Scene& m_scene;
  AbstractPropertyOwner::Kind m_allowed_kinds = AbstractPropertyOwner::Kind::All;
  AbstractPropertyOwner::Flag m_required_flags = AbstractPropertyOwner::Flag::Any;
  std::vector<AbstractPropertyOwner*> m_possible_references;

  std::vector<omm::AbstractPropertyOwner*> collect_candidates();
  void update_candidates();
};

}  // namespace omm
