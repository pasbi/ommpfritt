#include "tools/selectsimilartool.h"
#include "common.h"
#include "logging.h"
#include "objects/pathobject.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"
#include "properties/floatproperty.h"
#include "properties/optionproperty.h"
#include "properties/triggerproperty.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include "tools/handles/boundingboxhandle.h"

namespace
{
constexpr auto MODE_PROPERTY_KEY = "mode";
constexpr auto STRATEGY_PROPERTY_KEY = "strategy";
constexpr auto THRESHOLD_PROPERTY_KEY = "threshold";
constexpr auto APPLY_PROPERTY_KEY = "apply";
enum class Mode { Normal, X, Y, Distance };
enum class MatchStrategy { Any, All };
using namespace omm;

double normal_distance(const Point& a, const Point& b)
{
  const auto normalize = [](double a, double b) {
    return M_180_PI * PolarCoordinates::normalize_angle(std::abs(a - b)) - M_PI_2;
  };

  return std::min(normalize(a.left_tangent().argument, b.left_tangent().argument),
                  normalize(a.right_tangent().argument, b.right_tangent().argument));
}

omm::Vec2f distance(const PathObject& path_object,
                    const Point& a,
                    const Point& b,
                    omm::SelectSimilarTool::Alignment alignment)
{
  if (alignment == SelectSimilarTool::Alignment::Global) {
    const auto global_pos = [&path_object](const Point& point) {
      const auto t = path_object.global_transformation(omm::Space::Viewport);
      return t.apply_to_position(point.position());
    };
    return global_pos(a) - global_pos(b);
  } else {
    return a.position() - b.position();
  }
}

}  // namespace

namespace omm
{
SelectSimilarTool::SelectSimilarTool(Scene& scene) : SelectPointsBaseTool(scene)
{
  static constexpr double THRESHOLD_STEP = 0.1;
  const auto category = QObject::tr("tool");
  create_property<OptionProperty>(MODE_PROPERTY_KEY, 0)
      .set_options(
          {QObject::tr("normal"), QObject::tr("x"), QObject::tr("y"), QObject::tr("distance")})
      .set_label(QObject::tr("similarity"))
      .set_category(category)
      .set_animatable(false);
  create_property<OptionProperty>(STRATEGY_PROPERTY_KEY, 0)
      .set_options({QObject::tr("Any"), QObject::tr("All")})
      .set_label(QObject::tr("strategy"))
      .set_category(category)
      .set_animatable(false);
  create_property<FloatProperty>(THRESHOLD_PROPERTY_KEY, 1.0)
      .set_step(THRESHOLD_STEP)
      .set_label(QObject::tr("threshold"))
      .set_category(category)
      .set_animatable(false);
  create_property<TriggerProperty>(APPLY_PROPERTY_KEY)
      .set_label(QObject::tr("apply"))
      .set_category(category)
      .set_animatable(false);
  connect(&scene.mail_box(),
          &MailBox::point_selection_changed,
          this,
          &SelectSimilarTool::update_base_selection);
  update_property_appearance();
}

void SelectSimilarTool::reset()
{
  update_base_selection();
  SelectPointsBaseTool::reset();
  push_handle(std::make_unique<BoundingBoxHandle<SelectSimilarTool>>(*this));
}

void SelectSimilarTool::on_property_value_changed(Property* property)
{
  update_property_appearance();
  if (pmatch(property, {APPLY_PROPERTY_KEY})) {
    reset();
  } else if (pmatch(property, {THRESHOLD_PROPERTY_KEY, MODE_PROPERTY_KEY, STRATEGY_PROPERTY_KEY})) {
    update_selection();
  } else {
    SelectPointsBaseTool::on_property_value_changed(property);
  }
}

void SelectSimilarTool::update_selection()
{
  const auto strategy = property(STRATEGY_PROPERTY_KEY)->value<MatchStrategy>();
  for (const auto* path_object : scene()->item_selection<PathObject>()) {
    for (auto* point : path_object->geometry().points()) {
      if (m_base_selection.contains(point)) {
        const auto is_similar = [point, path_object, this](const PathPoint* b) {
          return this->is_similar(*path_object, point->geometry(), b->geometry());
        };
        const auto& begin = m_base_selection.begin();
        const auto& end = m_base_selection.end();
        switch (strategy) {
        case MatchStrategy::All:
          point->set_selected(std::all_of(begin, end, is_similar));
          break;
        case MatchStrategy::Any:
          point->set_selected(std::any_of(begin, end, is_similar));
          break;
        default:
          Q_UNREACHABLE();
        }
      }
    }
  }
  Q_EMIT scene()->mail_box().tool_appearance_changed(*this);
}

void SelectSimilarTool::update_base_selection()
{
  m_base_selection.clear();
  for (const auto* path_object : scene()->item_selection<PathObject>()) {
    for (auto* point : path_object->geometry().points()) {
      if (point->is_selected()) {
        m_base_selection.insert(point);
      }
    }
  }
}

bool SelectSimilarTool::is_similar(const PathObject& path_object, const Point& a, const Point& b) const
{
  const auto alignment = property(ALIGNMENT_PROPERTY_KEY)->value<Alignment>();
  const auto mode = property(MODE_PROPERTY_KEY)->value<Mode>();
  const auto distance = [mode, &path_object, alignment](const Point& a, const Point& b) {
    switch (mode) {
    case Mode::Normal:
      return normal_distance(a, b);
    case Mode::X:
      return std::abs(::distance(path_object, a, b, alignment).x);
    case Mode::Y:
      return std::abs(::distance(path_object, a, b, alignment).y);
    case Mode::Distance:
      return ::distance(path_object, a, b, alignment).euclidean_norm();
    default:
      Q_UNREACHABLE();
      return 0.0;
    }
  };

  const auto threshold = property(THRESHOLD_PROPERTY_KEY)->value<double>();
  return std::abs(distance(a, b)) < threshold;
}

void SelectSimilarTool::end()
{
  Q_EMIT scene()->mail_box().point_selection_changed();
}

void SelectSimilarTool::start()
{
  update_selection();
}

void SelectSimilarTool::update_property_appearance()
{
  const auto mode = property(MODE_PROPERTY_KEY)->value<Mode>();
  auto* const threshold_property = dynamic_cast<FloatProperty*>(property(THRESHOLD_PROPERTY_KEY));
  static const std::map<Mode, std::tuple<QString, double>> threshold_config{
      {Mode::Normal, {"°", 180.0}},  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
      {Mode::X, {"", std::numeric_limits<double>::max()}},
      {Mode::Y, {"", std::numeric_limits<double>::max()}},
      {Mode::Distance, {"", std::numeric_limits<double>::max()}},
  };
  const auto& [suffix, upper_limit] = threshold_config.at(mode);
  threshold_property->set_suffix(suffix);
  threshold_property->set_range(0.0, upper_limit);
}

}  // namespace omm
