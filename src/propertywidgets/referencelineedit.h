#pragma once

#include <QLineEdit>
#include "propertywidgets/multivalueedit.h"
#include "aspects/propertyowner.h"

class QMimeData;

namespace omm
{

class AbstractPropertyOwner;

class ReferenceLineEdit
  : public QLineEdit
  , public MultiValueEdit<AbstractPropertyOwner*>
{
  Q_OBJECT
public:
  explicit ReferenceLineEdit();
  void set_value(const value_type& value) override;
  value_type value() const override;

Q_SIGNALS:
  void reference_changed(AbstractPropertyOwner* reference);

protected:
  void set_inconsistent_value() override;
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
  bool can_drop(const QMimeData& mime_data) const;
  AbstractPropertyOwner* m_value;
};

}  // namespace omm
