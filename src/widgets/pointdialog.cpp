#include "widgets/pointdialog.h"
#include "objects/pathobject.h"
#include "path/pathvector.h"
#include "widgets/coordinateedit.h"
#include "widgets/pointedit.h"
#include <QComboBox>
#include <QDialogButtonBox>
#include <QScrollArea>
#include <QSettings>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <memory>

namespace
{
auto make_tab_widget_page(omm::PathObject& path_object, std::list<omm::PointEdit*>& point_edits)
{
  auto scroll_area = std::make_unique<QScrollArea>();
  scroll_area->setWidgetResizable(true);
  scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  auto widget = std::make_unique<QWidget>();
  auto layout = std::make_unique<QVBoxLayout>();

  for (auto* point : path_object.geometry().points()) {
    auto point_edit = std::make_unique<omm::PointEdit>(path_object, *point);
    point_edits.push_back(point_edit.get());
    layout->addWidget(point_edit.release());
  }
  layout->addStretch();

  widget->setLayout(layout.release());
  scroll_area->setWidget(widget.release());
  return scroll_area;
}

}  // namespace

namespace omm
{
PointDialog::PointDialog(const std::set<PathObject*>& paths, QWidget* parent) : QDialog(parent)
{
  assert(!paths.empty());
  auto tab_widget = std::make_unique<QTabWidget>();
  for (PathObject* path : paths) {
    tab_widget->addTab(make_tab_widget_page(*path, m_point_edits).release(), path->name());
  }
  auto button_box = std::make_unique<QDialogButtonBox>(QDialogButtonBox::Ok);
  connect(button_box.get(), &QDialogButtonBox::accepted, this, &PointDialog::accept);

  auto mode_combobox = std::make_unique<QComboBox>();
  m_mode_combobox = mode_combobox.get();
  m_mode_combobox->addItems({tr("Polar"), tr("Cartesian"), tr("Both")});
  connect(m_mode_combobox, qOverload<int>(&QComboBox::currentIndexChanged), [this](int i) {
    for (auto&& point_edit : m_point_edits) {
      point_edit->set_display_mode(static_cast<DisplayMode>(i + 1));
    }
  });
  QSettings settings;
  m_mode_combobox->setCurrentIndex(settings.value(DISPLAY_MODE_SETTINGS_KEY, 2).toInt());

  auto layout = std::make_unique<QVBoxLayout>();
  layout->addWidget(tab_widget.release());

  auto hlayout = std::make_unique<QHBoxLayout>();
  hlayout->addWidget(mode_combobox.release());
  hlayout->addWidget(button_box.release());

  layout->addLayout(hlayout.release());
  setLayout(layout.release());
}

PointDialog::~PointDialog()
{
  QSettings s;
  s.setValue(DISPLAY_MODE_SETTINGS_KEY, m_mode_combobox->currentIndex());
}

}  // namespace omm
