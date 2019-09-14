#pragma once

#include <QWidget>
#include "abstractfactory.h"
#include "properties/property.h"

class QComboBox;
class QLineEdit;
class QVBoxLayout;
class QFormLayout;

namespace omm
{

class PropertyConfigWidget
  : public QWidget
  , public AbstractFactory<std::string, PropertyConfigWidget>
{
  Q_OBJECT

public:
  explicit PropertyConfigWidget() = default;
  virtual void init(const Property::Configuration& configuration) = 0;
  virtual void update(Property::Configuration& configuration) const = 0;

protected:
  void hideEvent(QHideEvent *event) override;

Q_SIGNALS:
  void hidden();
};

void register_propertyconfigwidgets();

}  // namespace omm
