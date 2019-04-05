#include "widgets/pointdialog.h"
#include <QTabWidget>
#include "widgets/pointedit.h"
#include <memory>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include "objects/path.h"
#include <QScrollArea>
#include <QTimer>

namespace
{

auto make_tab_widget_page(const std::vector<omm::Point*>& points)
{
  auto scroll_area = std::make_unique<QScrollArea>();
  scroll_area->setWidgetResizable(true);
  scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  auto widget = std::make_unique<QWidget>();
  auto layout = std::make_unique<QVBoxLayout>();

  for (omm::Point* point : points) {
    layout->addWidget(std::make_unique<omm::PointEdit>(*point).release());
  }
  layout->addStretch();

  widget->setLayout(layout.release());
  scroll_area->setWidget(widget.release());
  return scroll_area;
}

}  // namespace

namespace omm
{

PointDialog::PointDialog(const std::set<Path*>& paths, QWidget* parent) : QDialog(parent)
{
  auto tab_widget = std::make_unique<QTabWidget>();
  for (Path* path : paths) {
    tab_widget->addTab( make_tab_widget_page(path->points()).release(),
                        QString::fromStdString(path->name()) );
  }
  auto button_box = std::make_unique<QDialogButtonBox>( QDialogButtonBox::Ok |
                                                        QDialogButtonBox::Cancel );
  connect(button_box.get(), SIGNAL(rejected()), this, SLOT(reject()));
  connect(button_box.get(), SIGNAL(accepted()), this, SLOT(accept()));

  auto layout = std::make_unique<QVBoxLayout>();
  layout->addWidget(tab_widget.release());
  layout->addWidget(button_box.release());
  setLayout(layout.release());
}

}  // namespace omm
