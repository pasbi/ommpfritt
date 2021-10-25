#include "mainwindow/toolbar/toolbardialog.h"
#include "keybindings/modeselector.h"
#include "main/application.h"
#include "mainwindow/toolbar/toolbar.h"
#include "mainwindow/toolbar/toolbaritemmodel.h"
#include "preferences/keybindingsproxymodel.h"
#include "preferences/preferencestree.h"
#include "preferences/preferencestreeitem.h"
#include "keybindings/keybindings.h"
#include "ui_toolbardialog.h"
#include <QIdentityProxyModel>
#include <QMimeData>
#include <QSortFilterProxyModel>

namespace
{
class DragDropProxy : public omm::KeyBindingsProxyModel
{
public:
  explicit DragDropProxy(omm::KeyBindings& key_bindings) : KeyBindingsProxyModel(key_bindings)
  {
  }

  [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override
  {
    Q_UNUSED(index);
    return Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable;
  }

  [[nodiscard]] Qt::DropActions supportedDragActions() const override
  {
    return Qt::LinkAction;
  }

  [[nodiscard]] Qt::DropActions supportedDropActions() const override
  {
    return Qt::IgnoreAction;
  }

  [[nodiscard]] int columnCount(const QModelIndex&) const override
  {
    return 1;
  }

  [[nodiscard]] QMimeData* mimeData(const QModelIndexList& indices) const override
  {
    nlohmann::json json;
    for (const QModelIndex& index : indices) {
      if (const QModelIndex sindex = mapToSource(index); sindex.isValid() && sindex.column() == 0) {
        const auto* ptr = static_cast<const omm::PreferencesTreeItem*>(sindex.internalPointer());
        if (!ptr->is_group()) {
          const auto* item = dynamic_cast<const omm::PreferencesTreeValueItem*>(ptr);
          assert(item->group == omm::Application::TYPE);
          json[omm::ToolBarItemModel::items_key].push_back(item->name.toStdString());
        }
      }
    }

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << QString::fromStdString(json.dump());

    auto mime_data = std::make_unique<QMimeData>();
    mime_data->setData(omm::ToolBarDialog::mime_type, data);
    return mime_data.release();
  }
};

}  // namespace

namespace omm
{
ToolBarDialog::ToolBarDialog(ToolBarItemModel& model, QWidget* parent)
    : QDialog(parent), m_key_bindings(*Application::instance().key_bindings),
      m_ui(std::make_unique<Ui::ToolBarDialog>()),
      m_proxy(std::make_unique<DragDropProxy>(m_key_bindings)), m_model(model)
{
  m_ui->setupUi(this);

  m_proxy->setSourceModel(&m_key_bindings);
  m_ui->tv_actions->setModel(m_proxy.get());

  const QModelIndex root = m_key_bindings.group_index(Application::TYPE);
  m_ui->tv_actions->setRootIndex(m_proxy->mapFromSource(root));

  m_ui->tv_actions->setDragEnabled(true);
  m_ui->tv_actions->setDragDropMode(QAbstractItemView::DragOnly);
  m_ui->tv_actions->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_ui->tv_actions->header()->hide();
  m_ui->tv_actions->setDropIndicatorShown(true);
  m_ui->tv_actions->setDefaultDropAction(Qt::LinkAction);

  connect(m_ui->pb_reset_filter, &QPushButton::clicked, [this]() {
    m_ui->le_name_filter->clear();
    m_ui->le_sequence_filter->clear();
  });

  connect(m_ui->le_name_filter,
          &QLineEdit::textChanged,
          m_proxy.get(),
          &DragDropProxy::set_action_name_filter);
  connect(m_ui->le_sequence_filter,
          &QKeySequenceEdit::keySequenceChanged,
          m_proxy.get(),
          &DragDropProxy::set_action_sequence_filter);
  connect(m_ui->pb_add_button, &QPushButton::clicked, &m_model, &ToolBarItemModel::add_group);
  connect(m_ui->pb_add_separator,
          &QPushButton::clicked,
          &m_model,
          &ToolBarItemModel::add_separator);
  connect(m_ui->pb_remove_items, &QPushButton::clicked, [this]() {
    m_model.remove_selection(m_ui->tv_toolbar->selectionModel()->selection());
  });

  {
    for (auto&& [name, mode_selector] : Application::instance().mode_selectors) {
      const auto tr_name = mode_selector->translated_name();
      auto action = std::make_unique<QAction>(tr_name);
      connect(action.get(), &QAction::triggered, this, [this, name = name]() {
        m_model.add_mode_selector(name);
      });
      m_ui->tb_add_switch->addAction(action.release());
    }
  }

  m_ui->tv_toolbar->setModel(&m_model);
  m_ui->tv_toolbar->setAcceptDrops(true);
  m_ui->tv_toolbar->setDragEnabled(true);
  m_ui->tv_toolbar->setAnimated(true);
  m_ui->tv_toolbar->setDropIndicatorShown(true);
  m_ui->tv_toolbar->setHeaderHidden(true);
  m_ui->tv_toolbar->setDragDropOverwriteMode(false);
  m_ui->tv_toolbar->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

ToolBarDialog::~ToolBarDialog() = default;

}  // namespace omm
