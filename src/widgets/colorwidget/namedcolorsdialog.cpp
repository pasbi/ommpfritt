#include "widgets/colorwidget/namedcolorsdialog.h"
#include "mainwindow/application.h"
#include "color/namedcolors.h"
#include "widgets/colorwidget/ui_namedcolorsdialog.h"

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
  m_ui->listView->edit(m_proxy->mapFromSource(index));
}

void NamedColorsDialog::remove()
{
  model().remove(m_proxy->mapToSource(m_ui->listView->currentIndex()));
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
