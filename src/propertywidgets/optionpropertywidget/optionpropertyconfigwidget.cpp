#include "propertywidgets/optionpropertywidget/optionpropertyconfigwidget.h"
#include "properties/optionproperty.h"
#include "logging.h"
#include <QEvent>
#include <QLayout>
#include <QListWidget>
#include <QMessageBox>
#include <QMouseEvent>
#include <functional>

namespace
{
std::unique_ptr<QListWidgetItem> make_item(const QString& label)
{
  auto item = std::make_unique<QListWidgetItem>(label);
  item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
  return item;
}

}  // namespace

namespace omm
{
static constexpr auto unnamed_option_label
    = QT_TRANSLATE_NOOP("OptionPropertyConfigWidget", "Unnamed Option");

OptionPropertyConfigWidget ::OptionPropertyConfigWidget()
{
  auto list_widget = std::make_unique<QListWidget>(this);
  m_list_widget = list_widget.get();
  m_list_widget->viewport()->installEventFilter(this);

  auto layout = std::make_unique<QVBoxLayout>();
  layout->addWidget(list_widget.release());
  setLayout(layout.release());
}

void OptionPropertyConfigWidget::init(const PropertyConfiguration& configuration)
{
  m_list_widget->clear();
  const auto items = configuration.get<std::deque<QString>>(OptionProperty::OPTIONS_POINTER, {});
  for (const QString& label : items) {
    m_list_widget->insertItem(m_list_widget->count(), make_item(label).release());
  }
  if (m_list_widget->count() == 0) {
    const QString label = tr(unnamed_option_label);
    m_list_widget->insertItem(m_list_widget->count(), make_item(label).release());
  }
}

void OptionPropertyConfigWidget::update(PropertyConfiguration& configuration) const
{
  std::deque<QString> items;
  const int n = m_list_widget->count();
  for (int row = 0; row < n; ++row) {
    const auto label = m_list_widget->item(row)->data(Qt::DisplayRole).toString();
    items.push_back(label);
  }
  configuration.set(OptionProperty::OPTIONS_POINTER, items);
}

void OptionPropertyConfigWidget::add_option(const QString& label)
{
  auto item = make_item(label);
  auto& ref = *item;
  m_list_widget->insertItem(m_list_widget->count(), item.release());
  m_list_widget->editItem(&ref);
}

void OptionPropertyConfigWidget::remove_option(int index)
{
  if (m_list_widget->count() > 1) {
    delete m_list_widget->takeItem(index);  // NOLINT(cppcoreguidelines-owning-memory)
  } else {
    LWARNING << "Prevented attempt to remove last option";
    QMessageBox::warning(this,
                         QObject::tr("Warning", "OptionPropertyConfigWidget"),
                         QObject::tr("Cannot remove last option.", "OptionPropertyConfigWidget"));
  }
}

bool OptionPropertyConfigWidget::eventFilter(QObject* watched, QEvent* event)
{
  const auto get_item = [event, this]() {
    const auto pos = dynamic_cast<QMouseEvent*>(event)->pos();
    return m_list_widget->itemAt(pos);
  };

  const auto attempt_remove_item = [get_item, this](QMouseEvent* event) {
    auto* item = get_item();
    if (event->button() == Qt::RightButton && item != nullptr) {
      remove_option(m_list_widget->row(item));
      return true;
    } else {
      return false;
    }
  };

  const auto attempt_add_item = [get_item, this](QMouseEvent* event) {
    auto* item = get_item();
    if (event->button() == Qt::LeftButton && item == nullptr) {
      add_option(tr(unnamed_option_label));
      return true;
    } else {
      return false;
    }
  };

  if (watched == m_list_widget->viewport()) {
    switch (event->type()) {
    case QEvent::MouseButtonPress:
      if (attempt_remove_item(dynamic_cast<QMouseEvent*>(event))) {
        return true;
      }
      break;
    case QEvent::MouseButtonDblClick:
      if (attempt_add_item(dynamic_cast<QMouseEvent*>(event))) {
        return true;
      }
      break;
    default:
      break;
    }
  }
  return PropertyConfigWidget::eventFilter(watched, event);
}

}  // namespace omm
