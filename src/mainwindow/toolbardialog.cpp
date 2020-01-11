#include "mainwindow/toolbardialog.h"
#include "mainwindow/application.h"
#include "ui_toolbardialog.h"
#include <QSortFilterProxyModel>
#include "preferences/keybindingsproxymodel.h"

namespace omm
{

ToolBarDialog::ToolBarDialog(const QString& tools, QWidget* parent)
  : QDialog(parent)
  , m_tools(tools)
  , m_key_bindings(Application::instance().key_bindings)
  , m_ui(std::make_unique<Ui::ToolBarDialog>())
  , m_filter_proxy(std::make_unique<KeyBindingsProxyModel>(m_key_bindings))
{
  m_ui->setupUi(this);

  m_ui->tv_actions->setSelectionBehavior(QAbstractItemView::SelectRows);

  m_filter_proxy->setSourceModel(&m_key_bindings);
  m_ui->tv_actions->setModel(m_filter_proxy.get());

  const QModelIndex root = m_key_bindings.group_index(Application::TYPE);
  m_ui->tv_actions->setRootIndex(m_filter_proxy->mapFromSource(root));

  connect(m_ui->pb_reset_filter, &QPushButton::clicked, [this]() {
    m_ui->le_name_filter->clear();
    m_ui->le_sequence_filter->clear();
  });
  connect(m_ui->le_name_filter, SIGNAL(textChanged(const QString&)),
          m_filter_proxy.get(), SLOT(set_action_name_filter(const QString&)));
  connect(m_ui->le_sequence_filter, SIGNAL(keySequenceChanged(const QKeySequence&)),
          m_filter_proxy.get(), SLOT(set_action_sequence_filter(const QKeySequence&)));

}

ToolBarDialog::~ToolBarDialog()
{
}

}  // namespace
