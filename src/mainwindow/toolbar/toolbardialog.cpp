#include "mainwindow/toolbar/toolbardialog.h"
#include "mainwindow/toolbar/toolbaritemmodel.h"
#include "mainwindow/toolbar/toolbar.h"
#include "mainwindow/application.h"
#include "ui_toolbardialog.h"
#include <QSortFilterProxyModel>
#include "preferences/keybindingsproxymodel.h"
#include <QIdentityProxyModel>
#include <QMimeData>

namespace
{

class DragDropProxy : public omm::KeyBindingsProxyModel
{
public:
  explicit DragDropProxy(omm::KeyBindings& key_bindings) : KeyBindingsProxyModel(key_bindings)
  {
  }

  Qt::ItemFlags flags(const QModelIndex& index) const override
  {
    Q_UNUSED(index);
    return Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable;
  }

  Qt::DropActions supportedDragActions() const override
  {
    return Qt::LinkAction;
  }

  Qt::DropActions supportedDropActions() const override
  {
    return Qt::IgnoreAction;
  }

  int columnCount(const QModelIndex&) const override { return 1; }

  QMimeData* mimeData(const QModelIndexList& indices) const override
  {
    std::list<const omm::PreferencesTreeValueItem*> items;
    for (const QModelIndex& index : indices) {
      if (const QModelIndex sindex = mapToSource(index); sindex.isValid() && sindex.column() == 0) {
        const auto* ptr = static_cast<const omm::PreferencesTreeItem*>(sindex.internalPointer());
        if (!ptr->is_group()) {
          items.push_back(static_cast<const omm::PreferencesTreeValueItem*>(ptr));
        }
      }
    }

    QStringList codes;
    for (const omm::PreferencesTreeValueItem* item : items) {
      assert(item->group == omm::Application::TYPE);
      codes.push_back(item->name);
    }

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << codes.join(omm::ToolBar::separator);

    auto mime_data = std::make_unique<QMimeData>();
    mime_data->setData(omm::ToolBarDialog::mime_type, data);
    return mime_data.release();
  }
};

}  // namespace

namespace omm
{

ToolBarDialog::ToolBarDialog(const QString& tools, QWidget* parent)
  : QDialog(parent)
  , m_key_bindings(Application::instance().key_bindings)
  , m_ui(std::make_unique<Ui::ToolBarDialog>())
  , m_proxy(std::make_unique<DragDropProxy>(m_key_bindings))
  , m_toolbar_item_model(std::make_unique<ToolBarItemModel>(tools))
{
  m_ui->setupUi(this);
  setWindowTitle(tr("Customize tool bar"));

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

  connect(m_ui->le_name_filter, SIGNAL(textChanged(const QString&)),
          m_proxy.get(), SLOT(set_action_name_filter(const QString&)));
  connect(m_ui->le_sequence_filter, SIGNAL(keySequenceChanged(const QKeySequence&)),
          m_proxy.get(), SLOT(set_action_sequence_filter(const QKeySequence&)));
  connect(m_ui->pb_add_button, SIGNAL(clicked()), m_toolbar_item_model.get(), SLOT(add_button()));
  connect(m_ui->pb_add_separator, SIGNAL(clicked()),
          m_toolbar_item_model.get(), SLOT(add_separator()));
  connect(m_ui->pb_remove_items, &QPushButton::clicked, [this]() {
    m_toolbar_item_model->remove_selection(m_ui->tv_toolbar->selectionModel()->selection());
  });

  m_ui->tv_toolbar->setModel(m_toolbar_item_model.get());
  m_ui->tv_toolbar->setAcceptDrops(true);
  m_ui->tv_toolbar->setDragEnabled(true);
  m_ui->tv_toolbar->setAnimated(true);
  m_ui->tv_toolbar->setDropIndicatorShown(true);
  m_ui->tv_toolbar->setHeaderHidden(true);
  m_ui->tv_toolbar->setDragDropOverwriteMode(false);
  m_ui->tv_toolbar->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

ToolBarDialog::~ToolBarDialog()
{
}

QString ToolBarDialog::tools() const
{
  return m_toolbar_item_model->encode();
}

}  // namespace
