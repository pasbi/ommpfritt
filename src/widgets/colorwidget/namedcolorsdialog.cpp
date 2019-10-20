#include "widgets/colorwidget/namedcolorsdialog.h"
#include "commands/propertycommand.h"
#include <QMessageBox>
#include "mainwindow/application.h"
#include "color/namedcolors.h"
#include "widgets/colorwidget/ui_namedcolorsdialog.h"
#include "properties/colorproperty.h"

namespace omm
{

NamedColorsDialog::NamedColorsDialog(QWidget* parent)
  : QDialog(parent), m_ui(new Ui::NamedColorsDialog)
{
  m_ui->setupUi(this);
  connect(m_ui->pb_add, SIGNAL(clicked()), this, SLOT(add()));
  connect(m_ui->pb_remove, SIGNAL(clicked()), this, SLOT(remove()));

  using ListViewNCHPM = NamedColorsHighlighProxyModel<QListView>;
  m_proxy = std::make_unique<ListViewNCHPM>(Application::instance().scene.named_colors(),
                                            *m_ui->listView);
  m_ui->listView->setModel(m_proxy.get());

  connect(m_ui->listView->selectionModel(),
          SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
          this, SLOT(setCurrent(const QModelIndex&)));
  connect(m_ui->w_colorwidget, &ColorWidget::color_changed, [this](const Color& color) {
    const QModelIndex index = m_ui->listView->currentIndex();
    if (index.isValid()) {
      model().set_color(m_proxy->mapToSource(index), color);
    }
  });

  m_ui->w_colorwidget->set_compact();
}

NamedColorsDialog::~NamedColorsDialog()
{
}

void NamedColorsDialog::add()
{
  QModelIndex index = model().add(m_ui->w_colorwidget->color());
  QModelIndex sindex = m_proxy->mapFromSource(index);
  m_ui->listView->setCurrentIndex(sindex);
  m_ui->listView->edit(sindex);
}

void NamedColorsDialog::remove()
{
  Scene& scene = Application::instance().scene;
  const QModelIndex index = m_proxy->mapToSource(m_ui->listView->currentIndex());
  if (index.isValid()) {
    const std::string name = model().name(index);
    auto properties = scene.find_named_color_holders(name);
    const std::size_t n = properties.size();
    bool can_remove = false;
    if (n > 0) {
      const QString msg = tr("%1 properties reference this named color.\n"
                             "Do you want to convert them into ordinary colors?").arg(n);
      const auto result = QMessageBox::question(this, tr("Convert Named Colors"), msg,
                                                QMessageBox::Yes | QMessageBox::No);
      can_remove = result == QMessageBox::Yes;
      for (ColorProperty* property : properties) {
        Color color = property->value();
        color.to_ordinary_color();
        using command_type = PropertiesCommand<ColorProperty>;
        scene.submit<command_type>(std::set<Property*> { property }, color);
      }
    } else {
      can_remove = true;
    }
    if (can_remove) {
      model().remove(index);
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
