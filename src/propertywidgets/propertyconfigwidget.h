#pragma once

#include "abstractfactory.h"
#include "properties/propertyconfiguration.h"
#include <QWidget>

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
  virtual void init(const PropertyConfiguration& configuration) = 0;
  virtual void update(PropertyConfiguration& configuration) const = 0;

protected:
  void hideEvent(QHideEvent* event) override;

Q_SIGNALS:
  void hidden();
};

template<typename PropertyT> class PropertyConfigWidget : public AbstractPropertyConfigWidget
{
public:
  static QString TYPE()
  {
    return QString(PropertyT::TYPE()) + "ConfigWidget";
  }
  [[nodiscard]] QString type() const override
  {
    return TYPE();
  }
};

}  // namespace omm
