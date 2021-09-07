#pragma once

#include <QObject>
#include <QEvent>
#include <QApplication>

class QAbstractButton;
class QTreeWidgetItem;

namespace omm
{

template<typename Observed>
class SyncPaletteDecorator : public QObject
{
protected:
  bool eventFilter(QObject* o, QEvent* e) override
  {
    if (o == QApplication::instance() && e->type() == QEvent::ApplicationPaletteChange) {
      update();
    }

    return QObject::eventFilter(o, e);
  }

  explicit SyncPaletteDecorator(QObject& parent, Observed& observed)
    : QObject(&parent)
    , m_observed(observed)
  {
    QApplication::instance()->installEventFilter(this);
  }

  Observed& m_observed;
  virtual void update() const = 0;
};

class SyncPaletteButtonDecorator : public SyncPaletteDecorator<QAbstractButton>
{
public:
  static void decorate(QAbstractButton& button);

private:
  using SyncPaletteDecorator::SyncPaletteDecorator;
  void update() const override;
};

class SyncPaletteTreeWidgetItemDecorator : public SyncPaletteDecorator<QTreeWidgetItem>
{
public:
  /**
   * @note The item must already be assigned to a `QTreeWidget` before calling this
   *  function and it must not be removed from that tree widget after that.
   */
  static void decorate(QTreeWidgetItem& item);

private:
  using SyncPaletteDecorator::SyncPaletteDecorator;
  void update() const override;
};

}  // namespace omm
