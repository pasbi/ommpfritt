#include "keybindings/actionwidget.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <memory>
#include "mainwindow/application.h"
#include <QApplication>

namespace omm
{

ActionWidget::ActionWidget(QWidget* parent, const PreferencesTreeValueItem& key_binding)
  : QWidget(parent)
{
  auto name_label = std::make_unique<QLabel>();
  m_name_label = name_label.get();
  name_label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  name_label->setContentsMargins(0, 0, 0, 0);

  auto icon_label = std::make_unique<QLabel>();
  icon_label->setFixedSize(12, 12);
  const QIcon icon = Application::instance().icon_provider.icon(key_binding.name);
  icon_label->setPixmap(icon.pixmap(icon_label->size()));

  auto shortcut_label = std::make_unique<QLabel>();
  m_shortcut_label = shortcut_label.get();
  shortcut_label->setAlignment(Qt::AlignRight);
  shortcut_label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  shortcut_label->setContentsMargins(0, 0, 0, 0);

  auto layout = std::make_unique<QHBoxLayout>();
  layout->addWidget(icon_label.release());
  layout->addWidget(name_label.release());
  layout->addSpacing(10);
  layout->addWidget(shortcut_label.release());
  layout->setContentsMargins(10, 3, 10, 3);

  setLayout(layout.release());
  setMouseTracking(true);
  connect(&key_binding, &PreferencesTreeValueItem::value_changed, this, [this](const std::string& s)
  {
    m_shortcut_label->setText(QKeySequence(QString::fromStdString(s)).toString(QKeySequence::NativeText));
    QResizeEvent resize_event(QSize(), m_shortcut_label->parentWidget()->size());
    qApp->sendEvent(m_shortcut_label->parent(), &resize_event);
    m_shortcut_label->parentWidget()->adjustSize();
    m_shortcut_label->parentWidget()->updateGeometry();
  });
  const QKeySequence sequence(QString::fromStdString(key_binding.value()));
  m_shortcut_label->setText(sequence.toString(QKeySequence::NativeText));
  m_name_label->setText(QString::fromStdString(key_binding.translated_name("keybindings")));
  setAutoFillBackground(true);
  set_highlighted(false);
}

void ActionWidget::set_highlighted(bool h)
{
  m_name_label->setForegroundRole(h ? QPalette::HighlightedText : QPalette::Text);
  m_shortcut_label->setForegroundRole(h ? QPalette::HighlightedText : QPalette::Text);
  setBackgroundRole(h ? QPalette::Highlight : QPalette::Base);
}

void ActionWidget::showEvent(QShowEvent* event)
{
  QResizeEvent resize_event(QSize(), parentWidget()->size());

  parentWidget()->adjustSize();
  qApp->sendEvent(parentWidget(), &resize_event);

  QWidget::showEvent(event);
}

}  // namespace
