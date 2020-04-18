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

class AbstractPropertyConfigWidget
  : public QWidget
  , public AbstractFactory<QString, false, AbstractPropertyConfigWidget>
{
  Q_OBJECT

public:
  explicit AbstractPropertyConfigWidget() = default;
  virtual void init(const Property::Configuration& configuration) = 0;
  virtual void update(Property::Configuration& configuration) const = 0;


protected:
  void hideEvent(QHideEvent *event) override;

Q_SIGNALS:
  void hidden();
};

template<typename PropertyT> class PropertyConfigWidget : public AbstractPropertyConfigWidget
{
public:
  static QString TYPE() { return QString(PropertyT::TYPE()) + "ConfigWidget"; }
  QString type() const override { return TYPE(); }
};

}  // namespace omm
