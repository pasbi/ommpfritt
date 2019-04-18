#include "keybindings/action.h"
#include <QLabel>
#include <memory>
#include <QHBoxLayout>
#include "keybindings/keybinding.h"
#include <QEvent>
#include <QWidget>
#include <QCoreApplication>
#include "common.h"
#include "logging.h"
#include "mainwindow/application.h"

namespace
{

class LiveLabel : public QLabel
{
public:
  LiveLabel(const std::function<std::string(void)>& getter) : m_getter(getter) {}
  QSize sizeHint() const override
  {
    const auto width = fontMetrics().width(QString::fromStdString(m_getter()));
    const auto height = fontMetrics().height();
    return QSize(width, height);
  }

protected:
  void paintEvent(QPaintEvent* event) override
  {
    setText(QString::fromStdString(m_getter()));
    QLabel::paintEvent(event);
  }

private:
  const std::function<std::string(void)> m_getter;
};

}  // namespace

namespace omm
{

class ActionWidget : public QWidget
{
public:
  ActionWidget(QWidget* parent, const omm::KeyBinding& key_binding) : QWidget(parent)
  {
    auto name_label = std::make_unique<LiveLabel>([&key_binding]() {
      const auto name = key_binding.name();
      return QCoreApplication::translate("any-context", name.c_str()).toStdString();
    });
    m_name_label = name_label.get();
    name_label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    name_label->setContentsMargins(0, 0, 0, 0);

    auto icon_label = std::make_unique<QLabel>();
    m_icon_label = icon_label.get();
    m_icon_label->setFixedSize(12, 12);
    m_icon_label->setPixmap(Application::instance().icon_provider.get_icon(key_binding.name(),
                                                                           m_icon_label->size()));

    auto shortcut_label = std::make_unique<LiveLabel>([&key_binding]() {
      const auto code = key_binding.key_sequence().toString().toStdString().c_str();
      return QCoreApplication::translate("any-context", code).toStdString();
    });
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
  }

  void set_highlighted(bool h)
  {
    m_name_label->setBackgroundRole(h ? QPalette::Highlight : QPalette::Window);
    m_name_label->setForegroundRole(h ? QPalette::HighlightedText : QPalette::Text);
    m_shortcut_label->setBackgroundRole(h ? QPalette::Highlight : QPalette::Window);
    m_shortcut_label->setForegroundRole(h ? QPalette::HighlightedText : QPalette::Text);
    setBackgroundRole(h ? QPalette::Highlight : QPalette::Window);
    setAutoFillBackground(h);
    m_name_label->setAutoFillBackground(h);
    m_shortcut_label->setAutoFillBackground(h);
  }

private:
  LiveLabel* m_name_label;
  LiveLabel* m_shortcut_label;
  QLabel* m_icon_label;

};

Action::Action(const KeyBinding& key_binding) : QWidgetAction(nullptr), m_key_binding(key_binding)
{
}

QWidget* Action::createWidget(QWidget* parent)
{
  auto aw = std::make_unique<ActionWidget>(parent, m_key_binding);
  m_action_widget = aw.get();
  return aw.release();
}

void Action::set_highlighted(bool h)
{
  if (m_action_widget != nullptr) { m_action_widget->set_highlighted(h); }
}

}  // namespace omm
