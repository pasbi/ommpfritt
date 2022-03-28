#include "managers/boundingboxmanager/boundingboxmanager.h"
#include "main/application.h"
#include "main/options.h"
#include "mainwindow/mainwindow.h"
#include "mainwindow/viewport/viewport.h"
#include "objects/pathobject.h"
#include "scene/mailbox.h"
#include "scene/pointselection.h"
#include "scene/scene.h"
#include "tools/toolbox.h"
#include "tools/tool.h"
#include "ui_boundingboxmanager.h"
#include <QComboBox>
#include <QLabel>

namespace
{
constexpr auto eps = 0.00001;

enum class AspectRatio { Ignore, FromWidth, FromHeight };

omm::ObjectTransformation find_transformation(const omm::BoundingBox& old_bb,
                                              const omm::BoundingBox& new_bb,
                                              AspectRatio aspect_ratio)
{
  omm::Vec2f s(1.0, 1.0);

  // if width (or height) of both bounding boxes are zero, the scale should be 1.0 rather than nan.
  static const auto get_scale = [](double old_s, double new_s) {
    if (std::abs(old_s) < eps && std::abs(new_s) < eps) {
      return 1.0;
    } else {
      return new_s / old_s;
    }
  };
  s.x = get_scale(old_bb.width(), new_bb.width());
  s.y = get_scale(old_bb.height(), new_bb.height());

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
    , m_transform_points_helper(scene, Space::Scene)
{
  auto widget = std::make_unique<QWidget>();
  m_ui->setupUi(widget.get());
  set_widget(std::move(widget));

  connect(&Application::instance().options(), &Options::anchor_changed, this, [this]() {
    reset_transformation();
  });

  const auto adjust_mode = [this](const Tool& tool) {
    if (tool.scene_mode() == SceneMode::Object) {
      m_current_mode = Mode::Objects;
    } else {
      m_current_mode = Mode::Points;
    }
    reset_transformation();
  };

  connect(&scene.tool_box(), &ToolBox::active_tool_changed, this, adjust_mode);
  adjust_mode(scene.tool_box().active_tool());

  connect(&scene.mail_box(), &MailBox::object_selection_changed, this, &BoundingBoxManager::update_manager);

  connect(&scene.mail_box(), &MailBox::object_appearance_changed, this, [this](Object& o) {
    if (o.type() == PathObject::TYPE) {
      update_manager();
    }
  });

  connect(&scene.mail_box(), &MailBox::point_selection_changed, this, &BoundingBoxManager::update_manager);
  connect(&scene.mail_box(), &MailBox::transformation_changed, this, &BoundingBoxManager::update_manager);

  static constexpr auto value_changed = &DoubleNumericEdit::value_changed;
  connect(m_ui->sp_x, value_changed, this, &BoundingBoxManager::update_bounding_box);
  connect(m_ui->sp_y, value_changed, this, &BoundingBoxManager::update_bounding_box);
  connect(m_ui->sp_w, value_changed, this, &BoundingBoxManager::update_bounding_box);
  connect(m_ui->sp_h, value_changed, this, &BoundingBoxManager::update_bounding_box);

  m_ui->sp_w->set_lower(0.0);
  m_ui->sp_h->set_lower(0.0);

  setFocusPolicy(Qt::StrongFocus);

  m_ui->sp_w->installEventFilter(this);
  m_ui->sp_h->installEventFilter(this);
  m_ui->sp_x->installEventFilter(this);
  m_ui->sp_y->installEventFilter(this);

  const auto update_spinbox_enabledness = [this]() {
    const auto bb = bounding_box();
    m_ui->sp_w->setEnabled(bb.width() > eps);
    m_ui->sp_h->setEnabled(bb.height() > eps);
  };

  static constexpr auto points_init_trans_changed = &TransformPointsHelper::initial_transformations_changed;
  connect(&m_transform_points_helper, points_init_trans_changed, this, update_spinbox_enabledness);

  static constexpr auto objects_init_trans_changed = &TransformObjectsHelper::initial_transformations_changed;
  connect(&m_transform_objects_helper, objects_init_trans_changed, this, update_spinbox_enabledness);
}

BoundingBoxManager::~BoundingBoxManager() = default;

QString BoundingBoxManager::type() const
{
  return TYPE;
}

void BoundingBoxManager::on_property_value_changed(Property& property)
{
  Q_UNUSED(property);
  reset_transformation();
}

BoundingBox BoundingBoxManager::update_manager()
{
  const BoundingBox bb = [this]() {
    switch (m_current_mode) {
    case Mode::Points:
      return BoundingBox{scene().point_selection->points(Space::Scene)};
    case Mode::Objects:
      return BoundingBox{
          util::transform(scene().item_selection<Object>(), [](const Object* o) {
            return o->recursive_bounding_box(o->global_transformation(Space::Scene));
          })};
    default:
      return BoundingBox();
    }
  }();

  auto blockers = acquire_signal_blockers();
  const auto& options = Application::instance().options();
  const Vec2f anchor = options.anchor_position(bb);
  m_ui->sp_x->set_value(anchor.x);
  m_ui->sp_y->set_value(anchor.y);
  m_ui->sp_w->set_value(bb.width());
  m_ui->sp_h->set_value(bb.height());

  if (bb.width() > eps) {
    m_ui->sp_w->setEnabled(true);
  }

  if (bb.height() > eps) {
    m_ui->sp_h->setEnabled(true);
  }

  return bb;
}

void BoundingBoxManager::update_bounding_box()
{
  auto blockers = acquire_signal_blockers();
  const BoundingBox new_bounding_box = bounding_box();

  if (m_old_bounding_box == new_bounding_box) {
    return;
  }

  const AspectRatio aspect_ratio = [&ui = m_ui]() {
    if (ui->cb_aspectratio->isChecked()) {
      if (ui->sp_h->hasFocus()) {
        return AspectRatio::FromHeight;
      } else if (ui->sp_w->hasFocus()) {
        return AspectRatio::FromWidth;
      }
    }
    return AspectRatio::Ignore;
  }();

  const ObjectTransformation t = find_transformation(m_old_bounding_box, new_bounding_box, aspect_ratio);
  switch (m_current_mode) {
  case Mode::Points:
    return scene().submit(m_transform_points_helper.make_command(t));
  case Mode::Objects:
    return scene().submit(m_transform_objects_helper.make_command(t.to_mat()));
  }
}

void BoundingBoxManager::reset_transformation()
{
  m_old_bounding_box = update_manager();
  switch (m_current_mode) {
  case Mode::Points:
    m_transform_points_helper.update(type_casts<PathObject*>(scene().item_selection<Object>()));
    break;
  case Mode::Objects:
    m_transform_objects_helper.update(scene().item_selection<Object>());
    break;
  }
}

std::vector<std::unique_ptr<QSignalBlocker>> BoundingBoxManager::acquire_signal_blockers()
{
  const auto os = {m_ui->sp_x, m_ui->sp_y, m_ui->sp_w, m_ui->sp_h};
  std::vector<std::unique_ptr<QSignalBlocker>> blockers;
  blockers.reserve(os.size());
  for (auto&& o : os) {
    blockers.push_back(std::make_unique<QSignalBlocker>(o));
  };
  return blockers;
}

BoundingBox BoundingBoxManager::bounding_box() const
{
  const Vec2f anchor(m_ui->sp_x->value(), m_ui->sp_y->value());
  const Vec2f size(m_ui->sp_w->value(), m_ui->sp_h->value());

  const Vec2f top_left = [size, anchor]() {
    switch (Application::instance().options().anchor()) {
    case Options::Anchor::TopLeft:
      return anchor;
    case Options::Anchor::TopRight:
      return anchor - Vec2f(size.x, 0.0);
    case Options::Anchor::BottomLeft:
      return anchor - Vec2f(0.0, size.y);
    case Options::Anchor::BottomRight:
      return anchor - size;
    case Options::Anchor::Center:
      return anchor - size / 2.0;
    default:
      Q_UNREACHABLE();
      return Vec2f();
    }
  }();

  return BoundingBox{top_left, top_left + size};
}

bool BoundingBoxManager::perform_action(const QString& name)
{
  LINFO << name;
  return false;
}

void BoundingBoxManager::enterEvent(QEvent* e)
{
  reset_transformation();
  Manager::enterEvent(e);
}

bool BoundingBoxManager::eventFilter(QObject* o, QEvent* e)
{
  if (o == m_ui->sp_h || o == m_ui->sp_w || o == m_ui->sp_x || o == m_ui->sp_y) {
    if (e->type() == QEvent::FocusOut) {
      reset_transformation();
    }
  }
  return Manager::eventFilter(o, e);
}

}  // namespace omm
