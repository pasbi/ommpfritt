#pragma once

#include <QLineEdit>
#include "propertywidgets/multivalueedit.h"
#include "properties/hasproperties.h"

class QMimeData;

namespace omm
{

class HasProperties;

class ReferenceLineEdit
  : public QLineEdit
  , public MultiValueEdit<HasProperties*>
{
  Q_OBJECT
public:
  explicit ReferenceLineEdit();
  void set_value(const value_type& value) override;
  value_type value() const override;

Q_SIGNALS:
  void reference_changed(HasProperties* reference);

protected:
  void set_inconsistent_value() override;
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;
private:
  bool can_drop(const QMimeData& mime_data) const;
  HasProperties* m_value;
};

}  // namespace omm
