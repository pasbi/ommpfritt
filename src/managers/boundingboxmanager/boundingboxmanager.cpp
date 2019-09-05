#include "managers/boundingboxmanager/boundingboxmanager.h"
#include "managers/boundingboxmanager/anchorwidget.h"
#include <QComboBox>
#include <QLabel>
#include "scene/scene.h"
#include "mainwindow/application.h"
#include "mainwindow/mainwindow.h"
#include "mainwindow/viewport/viewport.h"
#include "ui_boundingboxmanager.h"
#include "commands/objectstransformationcommand.h"

omm::ObjectTransformation
find_transformation(const omm::BoundingBox& a, const omm::BoundingBox& b,
                    omm::AnchorWidget::Anchor anchor, bool keep_aspect_ratio)
{
  omm::Vec2f s(b.width() / a.width(), b.height() / a.height());
  const omm::Vec2f ap = omm::AnchorWidget::anchor_position(a, anchor);
  const omm::Vec2f bp = omm::AnchorWidget::anchor_position(b, anchor);
  const omm::Vec2f t = bp - ap;

  if ((s - omm::Vec2f(1.0, 1.0)).euclidean_norm2() < t.euclidean_norm2()) {
    return omm::ObjectTransformation().translated(t);
  } else {
    if (keep_aspect_ratio) {
      if (std::abs(std::log(std::abs(s.x))) > std::abs(std::log(std::abs(s.y)))) {
        s.y = s.x;
      } else {
        s.x = s.y;
      }
    }
    omm::ObjectTransformation t;
    t = t.apply(omm::ObjectTransformation().translated(ap));
    t = t.apply(omm::ObjectTransformation().scaled(s));
    t = t.apply(omm::ObjectTransformation().translated(-ap));
    return t;
  }
}

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
    update_manager();
  });

  connect(m_ui->w_anchor, &AnchorWidget::anchor_changed, [this]() {
    update_manager();
  });

  const auto adjust_mode =  [this](const Tool& tool) {
    int index = tool.modifies_points() ? 0 : 1;
    m_ui->cb_mode->setCurrentIndex(index);
  };
  connect(&scene.tool_box, &ToolBox::active_tool_changed, adjust_mode);
  adjust_mode(scene.tool_box.active_tool());

  connect(&scene.message_box, SIGNAL(selection_changed(std::set<Object*>)),
          this, SLOT(update_manager()));
  QTimer::singleShot(1, [this]() {
    Viewport* viewport = &Application::instance().main_window()->viewport();
    assert(viewport != nullptr);
    connect(viewport, SIGNAL(updated()), this, SLOT(update_manager()));
  });

  connect(m_ui->sp_x, SIGNAL(value_changed()), this, SLOT(update_bounding_box()));
  connect(m_ui->sp_y, SIGNAL(value_changed()), this, SLOT(update_bounding_box()));
  connect(m_ui->sp_w, SIGNAL(value_changed()), this, SLOT(update_bounding_box()));
  connect(m_ui->sp_h, SIGNAL(value_changed()), this, SLOT(update_bounding_box()));

  m_ui->sp_w->set_lower(0.0);
  m_ui->sp_w->set_lower(0.0);
}

std::string BoundingBoxManager::type() const { return TYPE;  }

void BoundingBoxManager::on_property_value_changed(Property &property)
{
  Q_UNUSED(property);
  update_manager();
}

void BoundingBoxManager::update_manager()
{
  const BoundingBox bb = [this]() {
    switch (current_mode()) {
    case Mode::Points:
      return BoundingBox(::transform<Point, std::vector>(scene().point_selection.points()));
    case Mode::Objects:
      return BoundingBox(::transform<BoundingBox, std::vector>(scene().item_selection<Object>(),
                                                               [](const Object* o)
      {
        return o->recursive_bounding_box(o->global_transformation(true));
      }));
    default:
      return BoundingBox();
    }
  }();

  block_signals();
  const Vec2f anchor = m_ui->w_anchor->anchor_position(bb);
  m_ui->sp_x->set_value(anchor.x);
  m_ui->sp_y->set_value(anchor.y);
  m_ui->sp_w->set_value(bb.width());
  m_ui->sp_h->set_value(bb.height());
  unblock_signals();
  m_old_bounding_box = bb;
}

void BoundingBoxManager::update_bounding_box()
{
  switch (current_mode()) {
  case Mode::Points:
    return update_points();
  case Mode::Objects:
    return update_objects();
  }
}

void BoundingBoxManager::block_signals()
{
  m_ui->sp_x->blockSignals(true);
  m_ui->sp_y->blockSignals(true);
  m_ui->sp_w->blockSignals(true);
  m_ui->sp_h->blockSignals(true);
}

void BoundingBoxManager::unblock_signals()
{
  m_ui->sp_x->blockSignals(false);
  m_ui->sp_y->blockSignals(false);
  m_ui->sp_w->blockSignals(false);
  m_ui->sp_h->blockSignals(false);
}

BoundingBoxManager::Mode BoundingBoxManager::current_mode() const
{
  return static_cast<Mode>(m_ui->cb_mode->currentIndex());
}

void BoundingBoxManager::update_points()
{

}

BoundingBox BoundingBoxManager::bounding_box() const
{
  const Vec2f anchor(m_ui->sp_x->value(), m_ui->sp_y->value());
  const Vec2f size(m_ui->sp_w->value(), m_ui->sp_h->value());

  const Vec2f top_left = [size, anchor, this]() {
    switch (m_ui->w_anchor->anchor()) {
    case AnchorWidget::Anchor::TopLeft: return anchor;
    case AnchorWidget::Anchor::TopRight: return anchor - Vec2f(size.x, 0.0);
    case AnchorWidget::Anchor::BottomLeft: return anchor - Vec2f(0.0, size.y);
    case AnchorWidget::Anchor::BottomRight: return anchor - size;
    case AnchorWidget::Anchor::Center: return anchor - size/2.0;
    default:
      Q_UNREACHABLE();
      return Vec2f();
    }
  }();

  return BoundingBox(top_left, top_left + size);
}

void BoundingBoxManager::update_objects()
{
  block_signals();
  const BoundingBox new_bounding_box = bounding_box();

  if (m_old_bounding_box == new_bounding_box) {
    return;
  }

  const bool keep_aspect_ratio = m_ui->cb_aspectratio->isChecked();
  const ObjectTransformation t = find_transformation(m_old_bounding_box, new_bounding_box,
                                                     m_ui->w_anchor->anchor(), keep_aspect_ratio);
  auto objects = scene().item_selection<Object>();
  Object::remove_internal_children(objects);

  ObjectsTransformationCommand::Map transformations;
  for (Object* object : objects) {
    const auto to = [object, t]() {
      return t.apply(object->global_transformation(true));
    }();
    transformations.insert(std::pair(object, to));
  }

  if (transformations.size() > 0) {
    const auto mode = ObjectsTransformationCommand::TransformationMode::Object;
    auto command = std::make_unique<ObjectsTransformationCommand>(transformations, mode);
    scene().submit(std::move(command));
    Q_EMIT scene().message_box.appearance_changed();
  }

  m_old_bounding_box = new_bounding_box;
  unblock_signals();
}

void BoundingBoxManager::UiBoundingBoxManagerDeleter::operator()(Ui::BoundingBoxManager* ui)
{
  delete ui;
}

}  // namespace omm
