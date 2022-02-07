#include "mainwindow/toolbar/toolbaritemmodel.h"
#include "common.h"
#include "keybindings/modeselector.h"
#include "keybindings/keybindings.h"
#include "main/application.h"
#include "mainwindow/toolbar/toolbar.h"
#include "mainwindow/toolbar/toolbardialog.h"
#include "preferences/preferencestreeitem.h"
#include "logging.h"
#include <QMimeData>
#include <QToolButton>
#include <QTreeWidgetItem>
#include <QWidgetAction>

namespace
{
constexpr auto type_key = "type";
constexpr auto action_item_id = QStandardItem::UserType + 1;
constexpr auto group_item_id = QStandardItem::UserType + 2;
constexpr auto separator_item_id = QStandardItem::UserType + 3;
constexpr auto switch_item_id = QStandardItem::UserType + 4;

using namespace omm;

class AbstractItem : public QStandardItem
{
protected:
  using QStandardItem::QStandardItem;
  explicit AbstractItem() : QStandardItem(0, 1)
  {
    setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
  }

public:
  [[nodiscard]] virtual nlohmann::json encode() const = 0;
  [[nodiscard]] virtual std::unique_ptr<QAction> make_action() const = 0;
};

template<int item_id> class Item : public AbstractItem
{
public:
  static constexpr auto TYPE = item_id;
  [[nodiscard]] int type() const override
  {
    return TYPE;
  }

protected:
  using AbstractItem::AbstractItem;
};

class ActionItem : public Item<action_item_id>
{
public:
  explicit ActionItem(const QString& command_name) : command_name(command_name)
  {
    const auto& key_bindings = omm::Application::instance().key_bindings;
    const auto* item = key_bindings->value(omm::Application::TYPE, command_name);
    if (item == nullptr) {
      const auto d = ToolBarItemModel::tr("%1 is no valid command name.").arg(command_name);
      throw omm::ToolBarItemModel::BadConfigurationError(d);
    }
    setData(item->translated_name(), Qt::DisplayRole);
    setData(item->icon(), Qt::DecorationRole);
  }

  [[nodiscard]] std::unique_ptr<QAction> make_action() const override
  {
    auto& app = Application::instance();
    const auto& key_bindings = omm::Application::instance().key_bindings;
    return key_bindings->make_toolbar_action(app, command_name);
  }

  [[nodiscard]] nlohmann::json encode() const override
  {
    return command_name.toStdString();
  }

private:
  const QString command_name;
};

template<int type> class HyperItem : public Item<type>
{
public:
  void set_label(const QString& label)
  {
    this->setData(label, Qt::DisplayRole);
  }

protected:
  [[nodiscard]] nlohmann::json encode() const override
  {
    return {{type_key, type}};
  }
};

class GroupItem : public HyperItem<group_item_id>
{
public:
  explicit GroupItem()
  {
    set_label(omm::ToolBarItemModel::tr("group"));
    this->setFlags(this->flags() | Qt::ItemIsDropEnabled);
  }

  [[nodiscard]] nlohmann::json encode() const override
  {
    nlohmann::json items;
    for (int i = 0; i < rowCount(); ++i) {
      items.push_back(dynamic_cast<const AbstractItem*>(child(i, 0))->encode());
    }
    auto j = HyperItem::encode();
    j[omm::ToolBarItemModel::items_key] = items;
    return j;
  }

  [[nodiscard]] std::unique_ptr<QAction> make_action() const override
  {
    auto button = std::make_unique<QToolButton>();
    QObject::connect(button.get(),
                     &QToolButton::triggered,
                     button.get(),
                     &QToolButton::setDefaultAction);
    for (int row = 0; row < this->rowCount(); ++row) {
      const auto* item = this->child(row);
      button->addAction(dynamic_cast<const AbstractItem*>(item)->make_action().release());
    }
    if (button->actions().empty()) {
      LWARNING << "Skip empty group.";
      return nullptr;
    } else {
      button->setDefaultAction(button->actions().first());
      auto action = std::make_unique<QWidgetAction>(nullptr);
      action->setDefaultWidget(button.release());
      return action;
    }
  }
};

QString get_mode_selector(const nlohmann::json& item)
{
  using namespace omm;
  if (!item.contains("name")) {
    const auto msg = ToolBarItemModel::tr("Switch item has no mode selector.");
    throw ToolBarItemModel::BadConfigurationError(msg);
  }
  return QString::fromStdString(item["name"]);
}

class SwitchItem : public HyperItem<switch_item_id>
{
public:
  explicit SwitchItem(const nlohmann::json& item) : m_mode_selector(get_mode_selector(item))
  {
    const auto& mode_selector = *Application::instance().mode_selectors.at(m_mode_selector);
    set_label(mode_selector.translated_name());
  }

  [[nodiscard]] nlohmann::json encode() const override
  {
    auto j = HyperItem::encode();
    j["name"] = m_mode_selector.toStdString();
    return j;
  }

  [[nodiscard]] std::unique_ptr<QAction> make_action() const override
  {
    const auto& mode_selector = *Application::instance().mode_selectors.at(m_mode_selector);
    auto button = std::make_unique<QToolButton>();
    button->setPopupMode(QToolButton::InstantPopup);
    QObject::connect(button.get(),
                     &QToolButton::triggered,
                     button.get(),
                     &QToolButton::setDefaultAction);
    auto& app = Application::instance();
    const auto& key_bindings = omm::Application::instance().key_bindings;
    for (auto&& command_name : mode_selector.activation_actions) {
      auto action = key_bindings->make_toolbar_action(app, command_name);
      button->addAction(action.release());
    }
    button->setDefaultAction(button->actions()[mode_selector.mode()]);
    QObject::connect(&mode_selector,
                     &ModeSelector::mode_changed,
                     button.get(),
                     [button = button.get()](int mode) { button->actions().at(mode)->trigger(); });
    auto action = std::make_unique<QWidgetAction>(nullptr);
    action->setDefaultWidget(button.release());
    return action;
  }

private:
  const QString m_mode_selector;
};

class SeparatorItem : public HyperItem<separator_item_id>
{
public:
  explicit SeparatorItem()
  {
    set_label(omm::ToolBarItemModel::tr("separator"));
  }

  [[nodiscard]] std::unique_ptr<QAction> make_action() const override
  {
    auto action = std::make_unique<QAction>();
    action->setSeparator(true);
    return action;
  }
};

}  // namespace

namespace omm
{
nlohmann::json ToolBarItemModel::encode(const QModelIndexList& indices) const
{
  const auto filter = [indices](const QModelIndex& index) {
    const auto is_descendant_of = [index](QModelIndex potential_descendant) {
      while (true) {
        if (potential_descendant == index) {
          return true;
        } else if (potential_descendant.isValid()) {
          potential_descendant = potential_descendant.parent();
        } else {
          return false;
        }
      }
      Q_UNREACHABLE();
      return false;
    };
    return index.column() == 0 && std::none_of(indices.begin(), indices.end(), is_descendant_of);
  };

  nlohmann::json items;
  for (const QModelIndex& index : indices) {
    if (filter(index)) {
      continue;
    }
    items.push_back(dynamic_cast<const AbstractItem*>(itemFromIndex(index))->encode());
  }
  return {{items_key, items}};
}

QString ToolBarItemModel::encode_str(const QModelIndexList& indices) const
{
  return QString::fromStdString(encode(indices).dump());
}

QString ToolBarItemModel::encode_str() const
{
  return QString::fromStdString(encode().dump());
}

nlohmann::json ToolBarItemModel::encode() const
{
  if (const std::size_t n = rowCount(); n == 0) {
    return {};
  } else {
    return encode(QItemSelectionRange(index(0, 0), index(static_cast<int>(n) - 1, 0)).indexes());
  }
}

void ToolBarItemModel::remove_selection(const QItemSelection& selection)
{
  auto sorted_selection = selection;
  std::sort(sorted_selection.begin(),
            sorted_selection.end(),
            [](const QItemSelectionRange& a, const QItemSelectionRange& b) {
              if (a.top() < b.top()) {
                assert(a.bottom() < b.top());  // no overlap
                return false;
              } else {
                assert(a.top() > b.bottom());  // no overlap
                return true;
              }
            });
  for (const auto& range : sorted_selection) {
    beginRemoveRows(range.parent(), range.top(), range.bottom());
    removeRows(range.top(), range.height(), range.parent());
    endRemoveRows();
  }
}

void ToolBarItemModel::add_group()
{
  add_single_item({{type_key, GroupItem::TYPE}, {items_key, nlohmann::json::array()}});
}

void ToolBarItemModel::add_separator()
{
  add_single_item({{type_key, SeparatorItem::TYPE}});
}

void ToolBarItemModel::add_mode_selector(const QString& mode_selector_name)
{
  add_single_item({{type_key, SwitchItem::TYPE}, {"name", mode_selector_name.toStdString()}});
}

void ToolBarItemModel::add_items(const QString& code, int row, const QModelIndex& parent)
{
  try {
    const auto json = nlohmann::json::parse(code.toStdString());
    add_items(json, row, parent);
  } catch (const nlohmann::json::exception& e) {
    LWARNING << "Decode ToolBar: Failed to parse json document.";
    LINFO << e.what() << " " << code;
  }
}

void ToolBarItemModel::add_items(const nlohmann::json& code, int row, const QModelIndex& parent)
{
  QList<QStandardItem*> items;
  std::list<std::pair<GroupItem*, nlohmann::json>> groups;
  for (auto&& item : code.at(items_key)) {
    switch (item.type()) {
    case nlohmann::json::value_t::object:
      switch (static_cast<int>(item[type_key])) {
      case GroupItem::TYPE:
        if (parent.isValid()) {
          LWARNING << "Nested groups are not allowed in tool bars.";
          return;
        } else {
          auto group_item = std::make_unique<GroupItem>();
          groups.emplace_back(group_item.get(), item);
          items.push_back(group_item.release());
        }
        break;
      case SeparatorItem::TYPE:
        items.push_back(std::make_unique<SeparatorItem>().release());
        break;
      case SwitchItem::TYPE:
        items.push_back(std::make_unique<SwitchItem>(item).release());
        break;
      default:
        Q_UNREACHABLE();
      }
      break;
    case nlohmann::json::value_t::string:
      items.push_back(std::make_unique<ActionItem>(QString::fromStdString(item)).release());
      break;
    default:
      LWARNING << "ignoring data of unexpected type: " << QString::fromStdString(item.dump());
    }
  }

  row = row < 0 ? rowCount(parent) : row;
  beginInsertRows(parent, row, row + items.size() - 1);
  if (parent.isValid()) {
    itemFromIndex(parent)->insertRows(row, items);
  } else {
    for (int i = 0; i < items.size(); ++i) {
      insertRow(row + i, items[i]);
    }
  }
  endInsertRows();

  for (const auto& [group_item, configuration] : groups) {
    const QModelIndex index = indexFromItem(group_item);
    assert(index.isValid());
    add_items(configuration, 0, index);
  }
}

void ToolBarItemModel::populate(QToolBar& tool_bar) const
{
  for (int row = 0; row < rowCount(); ++row) {
    if (auto action = dynamic_cast<const AbstractItem*>(item(row, 0))->make_action(); action) {
      tool_bar.addAction(action.release());
    }
  }
}

void ToolBarItemModel::reset(const QString& configuration)
{
  beginResetModel();
  clear();
  add_items(configuration);
  endResetModel();
}

bool ToolBarItemModel::canDropMimeData(const QMimeData* data,
                                       Qt::DropAction action,
                                       [[maybe_unused]] int row,
                                       [[maybe_unused]] int column,
                                       [[maybe_unused]] const QModelIndex& parent) const
{
  if (data->hasFormat(ToolBarDialog::mime_type)) {
    return action == Qt::MoveAction || action == Qt::LinkAction;
  } else {
    return false;
  }
}

bool ToolBarItemModel::dropMimeData([[maybe_unused]] const QMimeData* data,
                                    [[maybe_unused]] Qt::DropAction action,
                                    [[maybe_unused]] int row,
                                    [[maybe_unused]] int column,
                                    const QModelIndex& parent)
{
  assert(canDropMimeData(data, action, row, column, parent));
  QDataStream stream(data->data(ToolBarDialog::mime_type));
  QString code;
  stream >> code;
  const auto json_code = nlohmann::json::parse(code.toStdString());
  add_items(json_code, row, parent);
  return false;
}

QMimeData* ToolBarItemModel::mimeData(const QModelIndexList& indices) const
{
  QByteArray buffer;
  {
    const auto json = encode(indices);
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    const auto code = QString::fromStdString(json.dump());
    stream << code;
  }
  auto mime_data = std::make_unique<QMimeData>();
  mime_data->setData(omm::ToolBarDialog::mime_type, buffer);
  return mime_data.release();
}

void ToolBarItemModel::add_single_item(const nlohmann::json& config)
{
  add_items(nlohmann::json{{items_key, {config}}}, rowCount());
}

ToolBarItemModel::BadConfigurationError::BadConfigurationError(const QString& description) noexcept
    : std::runtime_error(description.toStdString())
{
}

}  // namespace omm
