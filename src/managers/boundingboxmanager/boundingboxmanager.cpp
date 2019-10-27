#include "managers/boundingboxmanager/boundingboxmanager.h"
#include <QComboBox>
#include <QLabel>
#include "scene/scene.h"
#include "mainwindow/application.h"
#include "mainwindow/mainwindow.h"
#include "mainwindow/viewport/viewport.h"
#include "ui_boundingboxmanager.h"
#include "objects/path.h"
#include "tools/toolbox.h"
#include "scene/messagebox.h"
#include "mainwindow/application.h"

namespace {

static constexpr auto eps = 0.00001;

enum class AspectRatio { Ignore, FromWidth, FromHeight };

omm::ObjectTransformation
find_transformation(const omm::BoundingBox& old_bb, const omm::BoundingBox& new_bb,
                    AspectRatio aspect_ratio)
{
  omm::Vec2f s(1.0, 1.0);

  // if width (or height) of both bounding boxes are zero, the scale should be 1.0 rather than nan.
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

  const auto& options = omm::Application::instance().options();
  const omm::Vec2f ap = options.anchor_position(old_bb);
  const omm::Vec2f bp = options.anchor_position(new_bb);
  const omm::Vec2f t = bp - ap;

  if ((s - omm::Vec2f(1.0, 1.0)).euclidean_norm2() < t.euclidean_norm2()) {
    return omm::ObjectTransformation().translated(t);
  } else {
    switch (aspect_ratio) {
    case AspectRatio::FromWidth:
      s.y = s.x;
      break;
    case AspectRatio::FromHeight:
      s.x = s.y;
      break;
    case AspectRatio::Ignore:
      break;
    }

    omm::ObjectTransformation t;
    t = t.apply(omm::ObjectTransformation().translated(ap));
    t = t.apply(omm::ObjectTransformation().scaled(s));
    t = t.apply(omm::ObjectTransformation().translated(-ap));
    return t;
  }
}

}  // namespace

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

  connect(&Application::instance().options(), &Options::anchor_changed, [this]() {
    reset_transformation();
  });

  const auto adjust_mode =  [this](const Tool& tool) {
    if (tool.modifies_points()) {
      m_current_mode = Mode::Points;
    } else {
      m_current_mode = Mode::Objects;
    }
    reset_transformation();
  };

  connect(&scene.tool_box(), &ToolBox::active_tool_changed, this, adjust_mode);
  adjust_mode(scene.tool_box().active_tool());

  connect(&scene.message_box(), qOverload<const std::set<Object*>&>(&MessageBox::selection_changed),
          this, [this](const std::set<Object*>&)
  {
    update_manager();
  });

  connect(&scene.message_box(), qOverload<Object&>(&MessageBox::appearance_changed), this,
               [this](Object& o)
  {
    Path* path = type_cast<Path*>(&o);
    if (path != nullptr) {
       update_manager();
    }
  });

  connect(&scene.message_box(), &MessageBox::point_selection_changed, this, [this]() {
    update_manager();
  });

  connect(&scene.message_box(), &MessageBox::transformation_changed, this, [this]() {
    update_manager();
  });

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

QString BoundingBoxManager::type() const { return TYPE;  }

void BoundingBoxManager::on_property_value_changed(Property &property)
{
  Q_UNUSED(property);
  reset_transformation();
}

BoundingBox BoundingBoxManager::update_manager()
{
  const BoundingBox bb = [this]() {
    switch (m_current_mode) {
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
  const auto& options = Application::instance().options();
  const Vec2f anchor = options.anchor_position(bb);
  m_ui->sp_x->set_value(anchor.x);
  m_ui->sp_y->set_value(anchor.y);
  m_ui->sp_w->set_value(bb.width());
  m_ui->sp_h->set_value(bb.height());
  unblock_signals();

  if (m_ui->cb_aspectratio->isChecked()) {
    if (!m_ui->sp_w->hasFocus() && !m_ui->sp_h->hasFocus()) {
      m_ui->sp_w->setEnabled(bb.width() > eps);
      m_ui->sp_h->setEnabled(bb.height() > eps);
    }
  } else {
    if (!m_ui->sp_w->hasFocus()) {
      m_ui->sp_w->setEnabled(bb.width() > eps);
    }
    if (!m_ui->sp_h->hasFocus()) {
      m_ui->sp_h->setEnabled(bb.height() > eps);
    }
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

  const AspectRatio aspect_ratio = [&ui=m_ui]() {
    if (ui->cb_aspectratio->isChecked()) {
      if (ui->sp_h->hasFocus()) {
        return AspectRatio::FromHeight;
      } else if (ui->sp_w->hasFocus()) {
        return AspectRatio::FromWidth;
      }
    }
    return AspectRatio::Ignore;
  }();

  const ObjectTransformation t = find_transformation(m_old_bounding_box, new_bounding_box,
                                                     aspect_ratio );
  switch (m_current_mode) {
  case Mode::Points:
    scene().submit(m_transform_points_helper.make_command(t));
    break;
  case Mode::Objects:
    scene().submit(m_transform_objects_helper.make_command(t.to_mat()));
    break;
  }

  unblock_signals();
}

void BoundingBoxManager::reset_transformation()
{
  m_old_bounding_box = update_manager();
  switch (m_current_mode) {
  case Mode::Points:
    m_transform_points_helper.update(type_cast<Path*>(scene().item_selection<Object>()));
    break;
  case Mode::Objects:
    m_transform_objects_helper.update(scene().item_selection<Object>());
    break;
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

BoundingBox BoundingBoxManager::bounding_box() const
{
  const Vec2f anchor(m_ui->sp_x->value(), m_ui->sp_y->value());
  const Vec2f size(m_ui->sp_w->value(), m_ui->sp_h->value());

  const Vec2f top_left = [size, anchor]() {
    switch (Application::instance().options().anchor()) {
    case Options::Anchor::TopLeft: return anchor;
    case Options::Anchor::TopRight: return anchor - Vec2f(size.x, 0.0);
    case Options::Anchor::BottomLeft: return anchor - Vec2f(0.0, size.y);
    case Options::Anchor::BottomRight: return anchor - size;
    case Options::Anchor::Center: return anchor - size/2.0;
    default:
      Q_UNREACHABLE();
      return Vec2f();
    }
  }();

  return BoundingBox(top_left, top_left + size);
}

bool BoundingBoxManager::perform_action(const QString& name)
{
  LINFO << name;
  return false;
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
      m_ui->sp_h->setEnabled(m_ui->sp_h->value() > eps);
      m_ui->sp_w->setEnabled(m_ui->sp_w->value() > eps);
    }
  }
  return Manager::eventFilter(o, e);
}

void BoundingBoxManager::UiBoundingBoxManagerDeleter::operator()(Ui::BoundingBoxManager* ui)
{
  delete ui;
}

}  // namespace omm
