#include "widgets/colorwidget/namedcolorsdialog.h"
#include "scene/history/historymodel.h"
#include <QLineEdit>
#include "commands/propertycommand.h"
#include <QMessageBox>
#include "mainwindow/application.h"
#include "color/namedcolors.h"
#include "widgets/colorwidget/ui_namedcolorsdialog.h"
#include "properties/colorproperty.h"
#include "commands/namedcolorscommand.h"
#include <QStyledItemDelegate>

namespace
{

omm::Scene& scene() { return omm::Application::instance().scene; }

class NamedColorNameEditorDelegate : public QStyledItemDelegate
{
public:
  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                        const QModelIndex& index) const override
  {
    Q_UNUSED(option)
    Q_UNUSED(index)
    return new QLineEdit(parent);
  }

  void setEditorData(QWidget* editor, const QModelIndex& index) const override
  {
    static_cast<QLineEdit*>(editor)->setText(index.data(Qt::EditRole).toString());
  }

  void setModelData(QWidget* editor, QAbstractItemModel* model,
                    const QModelIndex& index) const override
  {
    Q_UNUSED(model);
    auto& proxy = static_cast<QAbstractProxyModel&>(*model);
    auto& named_colors = static_cast<omm::NamedColors&>(*proxy.sourceModel());
    const auto new_name = static_cast<QLineEdit&>(*editor).text();
    const auto old_name = named_colors.name(proxy.mapToSource(index));
    if (named_colors.has_color(new_name)) {
      LWARNING << "Failed to rename '" << old_name << "' to '"
               << new_name << "' to prevent name clash.";
    } else {
      const auto cprops = omm::Application::instance().scene.find_named_color_holders(old_name);
      const auto props = ::transform<omm::Property*>(cprops);

      auto cmd = std::make_unique<omm::ChangeNamedColorNameCommand>(old_name, new_name);
      auto macro = scene().history().start_macro(cmd->label());
      if (props.size() > 0) {
        scene().submit<omm::PropertiesCommand<omm::ColorProperty>>(props, omm::Color(new_name));
      }
      scene().submit(std::move(cmd));
    }
  }
};

}

namespace omm
{

NamedColorsDialog::NamedColorsDialog(QWidget* parent)
  : QDialog(parent), m_ui(new Ui::NamedColorsDialog), m_delegate(new NamedColorNameEditorDelegate)
{
  m_ui->setupUi(this);
  connect(m_ui->pb_add, SIGNAL(clicked()), this, SLOT(add()));
  connect(m_ui->pb_remove, SIGNAL(clicked()), this, SLOT(remove()));

  using ListViewNCHPM = NamedColorsHighlighProxyModel<QListView>;
  m_proxy = std::make_unique<ListViewNCHPM>(scene().named_colors(), *m_ui->listView);
  m_ui->listView->setModel(m_proxy.get());
  m_ui->listView->setItemDelegateForColumn(0, m_delegate.get());

  connect(m_ui->listView->selectionModel(),
          SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
          this, SLOT(setCurrent(const QModelIndex&)));
  connect(m_ui->w_colorwidget, &ColorWidget::color_changed, [this](const Color& color) {
    const QModelIndex index = m_ui->listView->currentIndex();
    if (index.isValid()) {
      const QString name = model().name(m_proxy->mapToSource(index));
      scene().submit<ChangeNamedColorColorCommand>(name, color);
    }
  });

  m_ui->w_colorwidget->set_compact();
}

NamedColorsDialog::~NamedColorsDialog()
{
}

void NamedColorsDialog::add()
{
  const Color color(Color::Model::RGBA, { 0.0, 0.0, 0.0, 0.0 });
  scene().submit<AddNamedColorCommand>(model().generate_default_name(), color);
}

void NamedColorsDialog::remove()
{
  const QModelIndex index = m_proxy->mapToSource(m_ui->listView->currentIndex());
  if (index.isValid()) {
    const QString name = model().name(index);
    auto properties = scene().find_named_color_holders(name);
    const std::size_t n = properties.size();
    bool can_remove = false;

    auto cmd = std::make_unique<RemoveNamedColorCommand>(model().name(index));
    if (n > 0) {
      const QString msg = tr("%n properties reference this named color.\n"
                             "Do you want to convert them into ordinary colors?",
                             "NamedColorsDialog",
                             n);
      const auto result = QMessageBox::question(this, tr("Convert Named Colors"), msg,
                                                QMessageBox::Yes | QMessageBox::No);
      can_remove = result == QMessageBox::Yes;
    } else {
      can_remove = true;
    }

    if (can_remove) {
      [[maybe_unused]] auto macro = scene().history().start_macro(cmd->label());
      for (ColorProperty* property : properties) {
        Color color = property->value();
        color.to_ordinary_color();
        using command_type = PropertiesCommand<ColorProperty>;
        scene().submit<command_type>(std::set<Property*> { property }, color);
      }
      scene().submit(std::move(cmd));
    }
  }
}

void NamedColorsDialog::setCurrent(const QModelIndex& index)
{
  if (index.isValid()) {
    assert(index.model() == m_ui->listView->model());
    QSignalBlocker blocker(m_ui->w_colorwidget);
    m_ui->w_colorwidget->set_color(model().color(m_proxy->mapToSource(index)));
  } else {
    LINFO << "invalid index";
  }
}

NamedColors& NamedColorsDialog::model() const
{
  return Application::instance().scene.named_colors();
}

}  // namespace omm
