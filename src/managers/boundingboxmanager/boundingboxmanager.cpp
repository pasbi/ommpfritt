#include "managers/boundingboxmanager/boundingboxmanager.h"
#include "managers/boundingboxmanager/anchorwidget.h"
#include <QComboBox>
#include <QLabel>
#include "scene/scene.h"
#include "mainwindow/application.h"
#include "mainwindow/mainwindow.h"
#include "mainwindow/viewport/viewport.h"

namespace omm
{

BoundingBoxManager::BoundingBoxManager(Scene& scene)
  : Manager(tr("Bounding Box Manager"), scene)
{
  setObjectName(TYPE);
  auto widget = std::make_unique<QWidget>();
  auto grid_layout = std::make_unique<QGridLayout>();
  auto hlayout = std::make_unique<QHBoxLayout>();
  auto layout = std::make_unique<QVBoxLayout>();

  auto pos_label = std::make_unique<QLabel>(tr("Pos:"));
  auto pos_x_field = std::make_unique<DoubleNumericEdit>();
  m_pos_x_field = pos_x_field.get();
  auto pos_y_field = std::make_unique<DoubleNumericEdit>();
  m_pos_y_field = pos_y_field.get();
  grid_layout->addWidget(pos_label.release(), 0, 0, Qt::AlignRight);
  grid_layout->addWidget(pos_x_field.release(), 0, 1);
  grid_layout->addWidget(pos_y_field.release(), 0, 2);

  auto size_label = std::make_unique<QLabel>(tr("Size:"));
  auto size_x_field = std::make_unique<DoubleNumericEdit>();
  m_size_x_field = size_x_field.get();
  auto size_y_field = std::make_unique<DoubleNumericEdit>();
  m_size_y_field = size_y_field.get();
  grid_layout->addWidget(size_label.release(), 1, 0, Qt::AlignRight);
  grid_layout->addWidget(size_x_field.release(), 1, 1);
  grid_layout->addWidget(size_y_field.release(), 1, 2);

  auto anchor_widget = std::make_unique<AnchorWidget>();
  m_anchor_widget = anchor_widget.get();
  grid_layout->addWidget(anchor_widget.release(), 0, 3, 2, 1);

  auto mode_combobox = std::make_unique<QComboBox>();
  mode_combobox->addItem(tr("Points"));
  mode_combobox->addItem(tr("Objects"));
  m_mode_combo_box = mode_combobox.get();
  hlayout->addWidget(mode_combobox.release());

  auto align_combobox = std::make_unique<QComboBox>();
  align_combobox->addItem(tr("World"));
  align_combobox->addItem(tr("Local"));
  m_align_combo_box = align_combobox.get();
  hlayout->addWidget(align_combobox.release());

  layout->addLayout(grid_layout.release());
  layout->addLayout(hlayout.release());
  widget->setLayout(layout.release());
  set_widget(std::move(widget));

  connect(m_mode_combo_box, qOverload<int>(&QComboBox::currentIndexChanged), [this]() {
    update_bounding_box();
  });

  connect(m_anchor_widget, &AnchorWidget::anchor_changed, [this]() {
    update_bounding_box();
  });

  connect(&scene.tool_box, &ToolBox::active_tool_changed, [this](const Tool& tool) {
    int index = tool.modifies_points() ? 0 : 1;
    m_mode_combo_box->setCurrentIndex(index);
  });

  connect(&scene.message_box, SIGNAL(selection_changed(std::set<Object*>)),
          this, SLOT(update_bounding_box()));
  QTimer::singleShot(1, [this]() {
    Viewport* viewport = &Application::instance().main_window()->viewport();
    assert(viewport != nullptr);
    connect(viewport, SIGNAL(updated()), this, SLOT(update_bounding_box()));
  });
}

std::string BoundingBoxManager::type() const { return TYPE;  }

void BoundingBoxManager::on_property_value_changed(Property &property)
{
  Q_UNUSED(property);
  update_bounding_box();
}

void BoundingBoxManager::update_bounding_box()
{
  if (current_mode() == Mode::Points) {
    const BoundingBox bb(::transform<Point, std::vector>(scene().point_selection.points()));
    const Vec2f anchor = m_anchor_widget->anchor_position(bb);
    m_pos_x_field->set_value(anchor.x);
    m_pos_y_field->set_value(anchor.y);
    m_size_x_field->set_value(bb.width());
    m_size_y_field->set_value(bb.height());
  }
}

BoundingBoxManager::Mode BoundingBoxManager::current_mode() const
{
  return static_cast<Mode>(m_mode_combo_box->currentIndex());
}

}  // namespace omm
