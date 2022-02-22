#include "propertywidgets/optionpropertywidget/optionsedit.h"
#include "common.h"
#include "logging.h"
#include "managers/nodemanager/nodeview.h"
#include <QAbstractItemView>
#include <QApplication>
#include <QGraphicsItem>
#include <QGraphicsProxyWidget>
#include <QLineEdit>
#include <QPaintEvent>
#include <QStylePainter>

namespace omm
{
PrefixComboBox::PrefixComboBox(QWidget* parent) : QComboBox(parent)
{
  view()->installEventFilter(this);
}

void OptionsEdit::set_value(const value_type& value)
{
  setCurrentIndex(value);
}
void OptionsEdit::set_inconsistent_value()
{
  setCurrentIndex(-1);
}
OptionsEdit::value_type OptionsEdit::value() const
{
  return currentIndex();
}

void OptionsEdit::wheelEvent(QWheelEvent* event)
{
  if (hasFocus()) {
    PrefixComboBox::wheelEvent(event);
  }
}

bool PrefixComboBox::eventFilter(QObject* o, QEvent* e)
{
  const bool handled = QComboBox::eventFilter(o, e);
  if (e->type() == QEvent::Hide && o == view()) {
    Q_EMIT popup_hidden();
  }
  return handled;
}

void PrefixComboBox::showPopup()
{
  if (!prevent_popup) {
    QComboBox::showPopup();
  }
  Q_EMIT popup_shown();
}

OptionsEdit::OptionsEdit(QWidget* parent) : PrefixComboBox(parent)
{
  setFocusPolicy(Qt::StrongFocus);
}

void OptionsEdit::set_options(const std::deque<QString>& options)
{
  clear();
  addItems(util::transform<QList>(options));
}

void PrefixComboBox::paintEvent(QPaintEvent*)
{
  QStylePainter painter(this);
  painter.setPen(palette().color(QPalette::Text));
  // draw the combobox frame, focusrect and selected etc.
  QStyleOptionComboBox opt;
  initStyleOption(&opt);
  painter.drawComplexControl(QStyle::CC_ComboBox, opt);
  // draw the icon and text
  opt.currentText = tr("%1: %2").arg(prefix, currentText());
  painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

}  // namespace omm
