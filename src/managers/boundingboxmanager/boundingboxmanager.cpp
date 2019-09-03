#include "managers/boundingboxmanager/boundingboxmanager.h"
#include "managers/boundingboxmanager/anchorwidget.h"
#include <QComboBox>
#include <QLabel>
#include "scene/scene.h"
#include "mainwindow/application.h"
#include "mainwindow/mainwindow.h"
#include "mainwindow/viewport/viewport.h"
#include "ui_boundingboxmanager.h"

namespace omm
{

BoundingBoxManager::BoundingBoxManager(Scene& scene)
  : Manager(tr("Bounding Box Manager"), scene)
  , m_ui(new ::Ui::BoundingBoxManager)
{
  setObjectName(TYPE);

  auto widget = std::make_unique<QWidget>();
  m_ui->setupUi(widget.get());
  set_widget(std::move(widget));

  connect(m_ui->cb_mode, qOverload<int>(&QComboBox::currentIndexChanged), [this]() {
    update_bounding_box();
  });

  connect(m_ui->cb_align, qOverload<int>(&QComboBox::currentIndexChanged), [this]() {
    update_bounding_box();
  });

  connect(m_ui->w_anchor, &AnchorWidget::anchor_changed, [this]() {
    update_bounding_box();
  });

  const auto adjust_mode =  [this](const Tool& tool) {
    int index = tool.modifies_points() ? 0 : 1;
    m_ui->cb_mode->setCurrentIndex(index);
  };
  connect(&scene.tool_box, &ToolBox::active_tool_changed, adjust_mode);
  adjust_mode(scene.tool_box.active_tool());

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
  const BoundingBox bb = [this, mode=current_mode()]() {
    switch (mode) {
    case Mode::Points:
      return BoundingBox(::transform<Point, std::vector>(scene().point_selection.points()));
    case Mode::Objects:
      return BoundingBox(::transform<BoundingBox, std::vector>(scene().item_selection<Object>(),
                                                               [this](const Object* o)
      {
        switch (current_align()) {
        case Align::Local:
          return o->recursive_bounding_box(o->transformation());
        case Align::Global:
          return o->recursive_bounding_box(o->global_transformation(true));
        default:
          return BoundingBox();
        }
      }));
    default:
      return BoundingBox();
    }
  }();

  const Vec2f anchor = m_ui->w_anchor->anchor_position(bb);
  m_ui->sp_x->set_value(anchor.x);
  m_ui->sp_y->set_value(anchor.y);
  m_ui->sp_w->set_value(bb.width());
  m_ui->sp_h->set_value(bb.height());
}

BoundingBoxManager::Mode BoundingBoxManager::current_mode() const
{
  return static_cast<Mode>(m_ui->cb_mode->currentIndex());
}

BoundingBoxManager::Align BoundingBoxManager::current_align() const
{
  return static_cast<Align>(m_ui->cb_align->currentIndex());
}

void BoundingBoxManager::UiBoundingBoxManagerDeleter::operator()(Ui::BoundingBoxManager* ui)
{
  delete ui;
}

}  // namespace omm
