#include "propertywidgets/optionspropertywidget/optionspropertyconfigwidget.h"
#include <functional>
#include <QLayout>
#include <QListWidget>
#include <QEvent>
#include <QMouseEvent>
#include <QMessageBox>

namespace omm
{

OptionsPropertyConfigWidget
::OptionsPropertyConfigWidget(QWidget* parent, Property& property)
  : PropertyConfigWidget(parent, property)
  , m_options_property(static_cast<OptionsProperty&>(property))
{
  auto list_widget = std::make_unique<QListWidget>(this);
  m_list_widget = list_widget.get();
  connect(m_list_widget, &QListWidget::itemChanged, [this]() {
    update_property_options();
  });

  m_list_widget->viewport()->installEventFilter(this);

  for (const std::string& option : m_options_property.options()) {
    m_list_widget->addItem(QString::fromStdString(option));
  }

  if (m_list_widget->count() == 0) {
    add_option("Unnamed Option");
  }

  box_layout()->addWidget(list_widget.release());
}

std::string OptionsPropertyConfigWidget::type() const
{
  return TYPE;
}

void OptionsPropertyConfigWidget::update_property_options()
{
  std::vector<std::string> options;
  options.reserve(m_list_widget->count());
  for (int i = 0; i < m_list_widget->count(); ++i) {
    options.push_back(m_list_widget->item(i)->text().toStdString());
  }
  m_options_property.set_options(options);
}

void OptionsPropertyConfigWidget::add_option(const std::string& label)
{
  auto item = std::make_unique<QListWidgetItem>(QString::fromStdString(label));
  item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
  auto& ref = *item;
  m_list_widget->addItem(item.release());
  m_list_widget->editItem(&ref);
  update_property_options();
}

void OptionsPropertyConfigWidget::remove_option(int index)
{
  if (m_list_widget->count() > 1) {
    delete m_list_widget->takeItem(index);
    update_property_options();
  } else {
    LOG(WARNING) << "Prevented attempt to remove last option";
    QMessageBox::warning(this, "Warning", "Cannot remove last option.");
  }
}

bool OptionsPropertyConfigWidget::eventFilter(QObject* watched, QEvent* event)
{
  const auto get_item = [event, this]() {
    const auto pos = static_cast<QMouseEvent*>(event)->pos();
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
      add_option("Unnamed Option");
      return true;
    } else {
      return false;
    }
  };

  if (watched == m_list_widget->viewport())
  {
    switch (event->type()) {
    case QEvent::MouseButtonPress:
      if (attempt_remove_item(static_cast<QMouseEvent*>(event))) {
        return true;
      }
      break;
    case QEvent::MouseButtonDblClick:
      if (attempt_add_item(static_cast<QMouseEvent*>(event))) {
        return true;
      }
      break;
    default:
      break;
    }
  }
  return PropertyConfigWidget::eventFilter(watched, event);
}

}  // namespace pmm

