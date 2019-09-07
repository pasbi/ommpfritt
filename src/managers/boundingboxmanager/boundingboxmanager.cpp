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
#include "objects/path.h"

omm::ObjectTransformation
find_transformation(const omm::BoundingBox& old_bb, const omm::BoundingBox& new_bb,
                    omm::AnchorWidget::Anchor anchor, bool keep_aspect_ratio)
{
  omm::Vec2f s(1.0, 1.0);

  // if width (or height) of both bounding boxes are zero, the scale should be 1.0 rather than nan.
  static constexpr double eps = 0.000001;
  if (std::abs(old_bb.width()) < eps) {
    assert(std::abs(new_bb.width()) < eps);
    s.x = 1.0;
  } else {
    s.x = new_bb.width() / old_bb.width();
  }
  if (std::abs(old_bb.height()) < eps) {
    assert(std::abs(new_bb.height()) < eps);
    s.y = 1.0;
  } else {
    s.y = new_bb.height() / old_bb.height();
  }

  const omm::Vec2f ap = omm::AnchorWidget::anchor_position(old_bb, anchor);
  const omm::Vec2f bp = omm::AnchorWidget::anchor_position(new_bb, anchor);
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
  , m_transform_points_helper(Space::Scene)
{
  setObjectName(TYPE);

  auto widget = std::make_unique<QWidget>();
  m_ui->setupUi(widget.get());
  set_widget(std::move(widget));

  connect(m_ui->cb_mode, qOverload<int>(&QComboBox::currentIndexChanged), [this]() {
    reset_transformation();
  });

  connect(m_ui->w_anchor, &AnchorWidget::anchor_changed, [this]() {
    reset_transformation();
  });

  const auto adjust_mode =  [this](const Tool& tool) {
    int index = tool.modifies_points() ? 0 : 1;
    m_ui->cb_mode->setCurrentIndex(index);
  };

  regc(connect(&scene.tool_box, &ToolBox::active_tool_changed, adjust_mode));
  adjust_mode(scene.tool_box.active_tool());

  regc(connect(&scene.message_box,
               qOverload<const std::set<Object*>&>(&MessageBox::selection_changed),
               [this](const std::set<Object*>&)
  {
    update_manager();
  }));

  regc(connect(&scene.message_box, qOverload<Object&>(&MessageBox::appearance_changed),
               [this](Object& o)
  {
    Path* path = type_cast<Path*>(&o);
    if (path != nullptr) {
       update_manager();
    }
  }));

  regc(connect(&scene.message_box, &MessageBox::point_selection_changed, [this]() {
    update_manager();
  }));

  connect(m_ui->sp_x, SIGNAL(value_changed()), this, SLOT(update_bounding_box()));
  connect(m_ui->sp_y, SIGNAL(value_changed()), this, SLOT(update_bounding_box()));
  connect(m_ui->sp_w, SIGNAL(value_changed()), this, SLOT(update_bounding_box()));
  connect(m_ui->sp_h, SIGNAL(value_changed()), this, SLOT(update_bounding_box()));

  m_ui->sp_w->set_lower(0.0);
  m_ui->sp_h->set_lower(0.0);

  setFocusPolicy(Qt::StrongFocus);

  m_ui->sp_w->installEventFilter(this);
  m_ui->sp_h->installEventFilter(this);
  m_ui->sp_x->installEventFilter(this);
  m_ui->sp_y->installEventFilter(this);

}

std::string BoundingBoxManager::type() const { return TYPE;  }

void BoundingBoxManager::on_property_value_changed(Property &property)
{
  Q_UNUSED(property);
  reset_transformation();
}

BoundingBox BoundingBoxManager::update_manager()
{
  const BoundingBox bb = [this]() {
    switch (current_mode()) {
    case Mode::Points:
      return BoundingBox(::transform<Point>(scene().point_selection.points(Space::Scene)));
    case Mode::Objects:
      return BoundingBox(::transform<BoundingBox>(scene().item_selection<Object>(),
                                                  [](const Object* o)
      {
        return o->recursive_bounding_box(o->global_transformation(Space::Scene));
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

  static constexpr auto eps = 0.00001;
  if (!m_ui->sp_w->hasFocus()) {
    m_ui->sp_w->setEnabled(bb.width() > eps);
  }

  if (!m_ui->sp_h->hasFocus()) {
    m_ui->sp_h->setEnabled(bb.height() > eps);
  }

  return bb;
}

void BoundingBoxManager::update_bounding_box()
{
  block_signals();
  const BoundingBox new_bounding_box = bounding_box();

  if (m_old_bounding_box == new_bounding_box) {
    return;
  }

  const bool keep_aspect_ratio = m_ui->cb_aspectratio->isChecked();

  const ObjectTransformation t = find_transformation(m_old_bounding_box, new_bounding_box,
                                                     m_ui->w_anchor->anchor(), keep_aspect_ratio);
  switch (current_mode()) {
  case Mode::Points:
    update_points(t);
    break;
  case Mode::Objects:
    update_objects(t);
    break;
  }

  unblock_signals();
}

void BoundingBoxManager::reset_transformation()
{
  m_old_bounding_box = update_manager();
  m_transform_points_helper.update(type_cast<Path*>(scene().item_selection<Object>()));
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

void BoundingBoxManager::enterEvent(QEvent *e)
{
  reset_transformation();
  Manager::enterEvent(e);
}

bool BoundingBoxManager::eventFilter(QObject *o, QEvent *e)
{
  if (o == m_ui->sp_h || o == m_ui->sp_w || o == m_ui->sp_x || o == m_ui->sp_y) {
    if (e->type() == QEvent::FocusOut) {
      reset_transformation();
    }
  }
  return Manager::eventFilter(o, e);
}

void BoundingBoxManager::update_points(const ObjectTransformation& t)
{
  auto cmd = m_transform_points_helper.make_command(t);
  scene().submit(std::move(cmd));
}

void BoundingBoxManager::update_objects(const ObjectTransformation& t)
{
  auto objects = scene().item_selection<Object>();
  Object::remove_internal_children(objects);

  ObjectsTransformationCommand::Map transformations;
  for (Object* object : objects) {
    const auto to = [object, t]() {
      return t.apply(object->global_transformation(Space::Scene));
    }();
    transformations.insert(std::pair(object, to));
  }

  if (transformations.size() > 0) {
    const auto mode = ObjectsTransformationCommand::TransformationMode::Object;
    auto command = std::make_unique<ObjectsTransformationCommand>(transformations, mode);
    scene().submit(std::move(command));
    Q_EMIT scene().message_box.appearance_changed();
  }
}

void BoundingBoxManager::UiBoundingBoxManagerDeleter::operator()(Ui::BoundingBoxManager* ui)
{
  delete ui;
}

}  // namespace omm
