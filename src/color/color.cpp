#include "color/color.h"

#include "color/namedcolors.h"
#include "logging.h"
#include "main/application.h"
#include "scene/scene.h"
#include "serializers/abstractdeserializer.h"
#include <QtGlobal>
#include <algorithm>
#include <cassert>
#include <cmath>

namespace
{
constexpr double HUE_RANGE_0_6 = 6.0;

std::array<double, 3> rgb_to_hsv(const std::array<double, 3>& rgb)
{
  const double r = std::clamp(rgb[0], 0.0, 1.0);
  const double g = std::clamp(rgb[1], 0.0, 1.0);
  const double b = std::clamp(rgb[2], 0.0, 1.0);

  const double cmax = std::max(r, std::max(g, b));
  const double cmin = std::min(r, std::min(g, b));
  const double delta = cmax - cmin;

  // calculate hue in the range [0-6]
  double hue_0_6 = [delta, cmax, r, g, b]() {
    double hue = 0.0;
    if (delta == 0.0) {
      hue = 0.0;
    } else if (cmax == r) {
      hue = std::fmod((g - b) / delta, HUE_RANGE_0_6);
    } else if (cmax == g) {
      static constexpr double HUE_OFFSET = 2.0;
      hue = ((b - r) / delta + HUE_OFFSET);
    } else if (cmax == b) {
      static constexpr double HUE_OFFSET = 4.0;
      hue = ((r - g) / delta + HUE_OFFSET);
    } else {
      Q_UNREACHABLE();
    }
    if (hue < 0.0) {
      hue += HUE_RANGE_0_6;
    }
    return hue;
  }();

  const double s = cmax == 0.0 ? 0.0 : delta / cmax;
  const double v = cmax;
  const double h = hue_0_6 / HUE_RANGE_0_6;

  return {h, s, v};
}

std::array<double, 3> hsv_to_rgb(const std::array<double, 3>& hsv)
{
  const double hue_0_6 = [h = hsv[0]]() mutable {
    h = std::fmod(h, 1.0);
    if (h < 0.0) {
      h += 1.0;
    }
    return h;
  }() * HUE_RANGE_0_6;

  const double c = hsv[2] * hsv[1];
  const double x = c * (1.0 - std::abs(std::fmod(hue_0_6, 2.0) - 1));
  const double m = hsv[2] - c;
  static constexpr int GREEN_RAISE = 0;
  static constexpr int RED_FALL = 1;
  static constexpr int BLUE_RAISE = 2;
  static constexpr int GREEN_FALL = 3;
  static constexpr int RED_RAISE = 4;
  static constexpr int BLUE_FALL = 5;
  switch (static_cast<int>(hue_0_6)) {
  case GREEN_RAISE:
    return {c + m, x + m, m};
  case RED_FALL:
    return {x + m, c + m, m};
  case BLUE_RAISE:
    return {m, c + m, x + m};
  case GREEN_FALL:
    return {m, x + m, c + m};
  case RED_RAISE:
    return {x + m, m, c + m};
  case BLUE_FALL:
    return {c + m, m, x + m};
  default:
    Q_UNREACHABLE();
    return {0.0, 0.0, 0.0};
  }
}

bool decode_hex(const QString& code, std::array<double, 4>& rgb)
{
  static const auto decode = [](const QString& code, int offset, double& v) {
    if (code.size() >= offset + 2) {
      bool ok = false;
      static constexpr int BASE_HEX = 16;
      static constexpr double MAX_8_BIT = 255.0;
      v = code.midRef(offset, 2).toInt(&ok, BASE_HEX) / MAX_8_BIT;
      return ok;
    } else {
      return false;
    }
  };

  static constexpr std::size_t RRGGBBAA_LENGTH = 9;
  static constexpr std::size_t RRGGBB_LENGTH = 7;
  static constexpr double DEFAULT_ALPHA_VALUE = 1.0;
  if (code.at(0) != '#' || (code.size() != RRGGBBAA_LENGTH && code.size() != RRGGBB_LENGTH)) {
    return false;
  } else {
    static constexpr std::size_t RR_OFFSET = 1;
    static constexpr std::size_t GG_OFFSET = 3;
    static constexpr std::size_t BB_OFFSET = 5;
    static constexpr std::size_t AA_OFFSET = 7;
    if (code.size() == RRGGBBAA_LENGTH || code.size() == RRGGBB_LENGTH) {
      if (!decode(code, RR_OFFSET, rgb[0]) || !decode(code, GG_OFFSET, rgb[1])
          || !decode(code, BB_OFFSET, rgb[2])) {
        return false;
      }
      if (code.size() == RRGGBBAA_LENGTH) {
        return decode(code, AA_OFFSET, rgb[3]);
      } else {
        rgb[3] = DEFAULT_ALPHA_VALUE;
        return true;
      }
      return true;
    } else {
      return false;
    }
  }
}

}  // namespace

namespace omm
{
Color::Color() : Color(Model::RGBA, {0.0, 0.0, 0.0, 1.0})
{
}

QString Color::component_name(const Color::Model& model, std::size_t component)
{
  static constexpr std::array<std::string_view, 4> HSVA_COMPONENT_NAMES{
      QT_TRANSLATE_NOOP("Color", "Hue"),
      QT_TRANSLATE_NOOP("Color", "Saturation"),
      QT_TRANSLATE_NOOP("Color", "Value"),
      QT_TRANSLATE_NOOP("Color", "Alpha"),
  };
  static constexpr std::array<std::string_view, 4> RGBA_COMPONENT_NAMES{
      QT_TRANSLATE_NOOP("Color", "Red"),
      QT_TRANSLATE_NOOP("Color", "Green"),
      QT_TRANSLATE_NOOP("Color", "Blue"),
      QT_TRANSLATE_NOOP("Color", "Alpha"),
  };
  switch (model) {
  case Model::HSVA:
    return HSVA_COMPONENT_NAMES.at(component).data();
  case Model::RGBA:
    return RGBA_COMPONENT_NAMES.at(component).data();
  case Model::Named:
    return "invalid (named)";
  default:
    Q_UNREACHABLE();
    return {};
  }
}

Color::Color(Color::Model model, const std::array<double, 3>& components, double alpha)
    : Color(model, {components[0], components[1], components[2], alpha})
{
}

Color::Color(Color::Model model, const std::array<double, 4>& components)
    : m_components(components), m_current_model(model)
{
}

Color::Color(const QString& name)
    : m_components({0.0, 0.0, 0.0, 0.0}), m_current_model(Model::Named), m_name(name)
{
}

Color::Color(const QColor& c) : Color(Color::RGBA, {c.redF(), c.greenF(), c.blueF(), c.alphaF()})
{
}

std::array<double, 4>
Color::convert(Color::Model from, Color::Model to, const std::array<double, 4>& values)
{
  static const auto pack = [](const std::array<double, 3>& triple, double fourth) {
    return std::array<double, 4>{triple[0], triple[1], triple[2], fourth};
  };
  static constexpr std::array<double, 4> invalid{0.0, 0.0, 0.0, 0.0};
  switch (from) {
  case Model::HSVA:
    switch (to) {
    case Model::HSVA:
      return values;
    case Model::RGBA:
      return pack(hsv_to_rgb({values[0], values[1], values[2]}), values[3]);
    default:
      Q_UNREACHABLE();
      return invalid;
    }
  case Model::RGBA:
    switch (to) {
    case Model::HSVA:
      return pack(rgb_to_hsv({values[0], values[1], values[2]}), values[3]);
    case Model::RGBA:
      return values;
    default:
      Q_UNREACHABLE();
      return invalid;
    }
  default:
    Q_UNREACHABLE();
    return invalid;
  }
}

void Color::convert(Color::Model to)
{
  assert(to != Color::Model::Named);
  to_ordinary_color();
  m_components = convert(m_current_model, to, m_components);
  m_current_model = to;
}

Color Color::convert(Color::Model to) const
{
  Color copy = *this;
  copy.convert(to);
  return copy;
}

std::array<double, 4> Color::components(Model model) const
{
  return convert(model).m_components;
}

void Color::set_components(Color::Model model, const std::array<double, 4>& components)
{
  m_current_model = model;
  m_components = components;
}

double& Color::component(Color::Role role)
{
  assert(model(role, m_current_model) == m_current_model);
  switch (role) {
  case Role::Red:
    [[fallthrough]];
  case Role::Hue:
    return m_components[0];
  case Role::Green:
    [[fallthrough]];
  case Role::Saturation:
    return m_components[1];
  case Role::Blue:
    [[fallthrough]];
  case Role::Value:
    return m_components[2];
  case Role::Alpha:
    return m_components[3];
  default:
    Q_UNREACHABLE();
    return m_components[0];
  }
}

Color::Model Color::model(Role role, Model tie)
{
  switch (role) {
  case Role::Red:  // NOLINT(bugprone-branch-clone)
    [[fallthrough]];
  case Role::Green:
    [[fallthrough]];
  case Role::Blue:
    return Model::RGBA;
  case Role::Hue:  // NOLINT(bugprone-branch-clone)
    [[fallthrough]];
  case Role::Saturation:
    [[fallthrough]];
  case Role::Value:
    return Model::HSVA;
  case Role::Alpha:
    if (tie == Model::Named) {
      return Model::RGBA;
    } else {
      return tie;
    }
  default:
    Q_UNREACHABLE();
    return Model::RGBA;
  }
}

QString Color::to_html() const
{
  static const auto to_hex = [](const double f) {
    const auto i = std::clamp(static_cast<int>(std::round(f * 255.0)), 0, 255);
    static constexpr auto base_hex = 16;
    auto str = QString("%1").arg(static_cast<int>(i), 2, base_hex, QChar('0'));
    assert(str.size() == 2);
    return str;
  };

  const auto rgba = components(Model::RGBA);
  return "#" + to_hex(rgba[0]) + to_hex(rgba[1]) + to_hex(rgba[2]) + to_hex(rgba[3]);
}

Color Color::from_html(const QString& html, bool* ok)
{
  Color color(Model::RGBA, {0.0, 0.0, 0.0, 1.0});
  if (decode_hex(html, color.m_components)) {
    if (ok != nullptr) {
      *ok = true;
    }
  } else {
    if (ok != nullptr) {
      *ok = false;
    }
  }
  return color;
}

Color Color::from_qcolor(const QColor& color)
{
  return Color(Color::Model::RGBA, {color.redF(), color.greenF(), color.blueF(), color.alphaF()});
}

QColor Color::to_qcolor() const
{
  QColor qc;
  const auto rgba = components(Model::RGBA);
  qc.setRedF(std::clamp(rgba[0], 0.0, 1.0));
  qc.setGreenF(std::clamp(rgba[1], 0.0, 1.0));
  qc.setBlueF(std::clamp(rgba[2], 0.0, 1.0));
  qc.setAlphaF(std::clamp(rgba[3], 0.0, 1.0));
  return qc;
}

void Color::to_ordinary_color()
{
  if (model() == Model::Named) {
    if (!Application::instance().scene->named_colors().resolve(m_name, *this)) {
      m_components = {0, 0, 0, 1};
      m_current_model = Model::RGBA;
      LWARNING << "Failed to resolve color '" << m_name << "'.";
    }
  }
  m_name.clear();
}

QString Color::name() const
{
  if (model() == Model::Named) {
    return m_name;
  } else {
    LWARNING << "requested name of ordinary color.";
    return "";
  }
}

void Color::set(Color::Role role, double value)
{
  to_ordinary_color();
  switch (role) {
  case Role::Red:  // NOLINT(bugprone-branch-clone)
    [[fallthrough]];
  case Role::Green:
    [[fallthrough]];
  case Role::Blue:
    convert(Model::RGBA);
    component(role) = value;
    return;
  case Role::Hue:  // NOLINT(bugprone-branch-clone)
    [[fallthrough]];
  case Role::Saturation:
    [[fallthrough]];
  case Role::Value:
    convert(Model::HSVA);
    component(role) = value;
    return;
  case Role::Alpha:
    component(role) = value;
    return;
  default:
    Q_UNREACHABLE();
  }
}

double Color::get(Color::Role role) const
{
  Color copy = *this;
  copy.to_ordinary_color();
  copy.convert(model(role, m_current_model));
  return copy.component(role);
}

bool operator==(const Color& a, const Color& b)
{
  if (a.m_current_model == b.m_current_model) {
    if (a.m_current_model == Color::Model::Named) {
      return a.m_name == b.m_name;
    } else {
      return a.m_components == b.m_components;
    }
  } else {
    return false;
  }
}

bool operator!=(const Color& a, const Color& b)
{
  return !(a == b);
}

bool operator<(const Color& a, const Color& b)
{
  if (a.m_current_model < b.m_current_model) {
    return true;
  } else if (a.m_current_model > b.m_current_model) {
    return false;
  } else {
    if (a.m_current_model == Color::Model::Named) {
      assert(b.m_current_model == Color::Model::Named);
      return a.m_name < b.m_name;
    } else {
      return a.m_components < b.m_components;  // NOLINT(modernize-use-nullptr)
    }
  }
}

QString Color::to_string() const
{
  const QString id = [this]() {
    if (model() == Color::Model::Named) {
      return name();
    } else {
      QStringList cs;
      const auto components = this->components(model());
      for (std::size_t i = 0; i < components.size(); ++i) {
        const auto component_name = Color::component_name(model(), i);
        cs.append(QString("%2: %1").arg(components.at(i)).arg(component_name));
      }
      return cs.join(", ");
    }
  }();
  return QString("Color[%1]").arg(id);
}

void Color::serialize(serialization::SerializerWorker& worker) const
{
  if (model() == Color::Model::Named) {
    worker.sub("name")->set_value(name());
    worker.sub("rgba")->set_value(std::vector{0.0, 0.0, 0.0, 0.0});
  } else {
    worker.sub("name")->set_value(QString{});
    worker.sub("rgba")->set_value(components(Color::Model::RGBA));
  }
}

void Color::deserialize(serialization::DeserializerWorker& worker)
{
  try {
    const auto n = worker.sub("name")->get_string();
    const auto v = worker.sub("rgba")->get<std::vector<double>>();
    if (n.isEmpty()) {
      m_current_model = Color::Model::RGBA;
      m_components = {v.at(0), v.at(1), v.at(2), v.at(3)};
    } else {
      m_current_model = Color::Model::Named;
      m_name = n;
    }
  } catch (std::out_of_range&) {
    throw omm::serialization::AbstractDeserializer::DeserializeError("Expected vector of size 4.");
  }
}

}  // namespace omm
