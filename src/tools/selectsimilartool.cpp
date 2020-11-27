#include "tools/selectsimilartool.h"
#include "logging.h"
#include "objects/path.h"
#include "properties/floatproperty.h"
#include "properties/optionproperty.h"
#include "properties/triggerproperty.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include "tools/handles/boundingboxhandle.h"
#include "common.h"

namespace
{
constexpr auto MODE_PROPERTY_KEY = "mode";
constexpr auto STRATEGY_PROPERTY_KEY = "strategy";
constexpr auto THRESHOLD_PROPERTY_KEY = "threshold";
constexpr auto APPLY_PROPERTY_KEY = "apply";
enum class Mode { Normal, X, Y, Distance };
enum class MatchStrategy { Any, All };
using It = omm::Path::iterator;

double normal_distance(const It& a, const It& b)
{
  const auto normalize = [](double a, double b) {
    return omm::M_180_PI * omm::PolarCoordinates::normalize_angle(std::abs(a - b)) - M_PI_2;
  };

  return std::min(normalize(a->left_tangent.argument, b->left_tangent.argument),
                  normalize(a->right_tangent.argument, b->right_tangent.argument));
}

omm::Vec2f distance(const It& a, const It& b, omm::SelectSimilarTool::Alignment alignment)
{
  if (alignment == omm::SelectSimilarTool::Alignment::Global) {
    const auto global_pos = [](const It& it) {
      const auto t = it.path->global_transformation(omm::Space::Viewport);
      return t.apply_to_position(it->position);
    };
    return global_pos(a) - global_pos(b);
  } else {
    return a->position - b->position;
  }
}

}  // namespace

namespace omm
{
SelectSimilarTool::SelectSimilarTool(Scene& scene) : SelectPointsBaseTool(scene)
{
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
      .set_step(0.1)
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
  handles.clear();
  make_handles(*this, false);
  handles.push_back(std::make_unique<BoundingBoxHandle<SelectSimilarTool>>(*this));
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
  for (auto&& path : scene()->item_selection<Path>()) {
    for (auto&& it = path->begin(); it != path->end(); ++it) {
      if (!::contains(m_base_selection, it)) {
        it->is_selected = [this, strategy, &it] {
          const auto is_similar = [&it, this](auto&& b) { return this->is_similar(it, b); };
          switch (strategy) {
          case MatchStrategy::All:
            return std::all_of(m_base_selection.begin(), m_base_selection.end(), is_similar);
          case MatchStrategy::Any:
            return std::any_of(m_base_selection.begin(), m_base_selection.end(), is_similar);
          default:
            Q_UNREACHABLE();
            return false;
          }
        }();
      }
    }
  }
  Q_EMIT scene()->mail_box().tool_appearance_changed(*this);
}

void SelectSimilarTool::update_base_selection()
{
  m_base_selection.clear();
  for (auto&& path : scene()->item_selection<Path>()) {
    for (auto it = path->begin(); it != path->end(); ++it) {
      if (it->is_selected) {
        m_base_selection.insert(it);
      }
    }
  }
}

bool SelectSimilarTool::is_similar(const Path::iterator& a, const Path::iterator& b) const
{
  using It = Path::iterator;
  const auto alignment = property(ALIGNMENT_PROPERTY_KEY)->value<Alignment>();
  LINFO << (int)alignment << " " << (int)Alignment::Global << " " << (int)Alignment::Local;
  const std::map<Mode, std::function<double(const It&, const It)>> mode_map{
      {Mode::Normal, normal_distance},
      {Mode::X,
       [alignment](auto&& a, auto&& b) { return std::abs(::distance(a, b, alignment).x); }},
      {Mode::Y,
       [alignment](auto&& a, auto&& b) { return std::abs(::distance(a, b, alignment).y); }},
      {
          Mode::Distance,
          [alignment](auto&& a, auto&& b) { return ::distance(a, b, alignment).euclidean_norm(); },
      },
  };

  const auto mode = property(MODE_PROPERTY_KEY)->value<Mode>();
  const auto threshold = property(THRESHOLD_PROPERTY_KEY)->value<double>();
  return std::abs(mode_map.at(mode)(a, b)) < threshold;
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
  auto* const threshold_property = static_cast<FloatProperty*>(property(THRESHOLD_PROPERTY_KEY));
  static const std::map<Mode, std::tuple<QString, double>> threshold_config{
      {Mode::Normal, {"°", 180.0}},  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
      {Mode::X, {"", std::numeric_limits<double>::max()}},
      {Mode::Y, {"", std::numeric_limits<double>::max()}},
      {Mode::Distance, {"", std::numeric_limits<double>::max()}},
  };
  const auto [suffix, upper_limit] = threshold_config.at(mode);
  threshold_property->set_suffix(suffix);
  threshold_property->set_range(0.0, upper_limit);
}

}  // namespace omm
