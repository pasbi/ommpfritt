#include "color/color.h"

#include "color/namedcolors.h"
#include "logging.h"
#include "mainwindow/application.h"
#include <QtGlobal>
#include <algorithm>
#include <cassert>
#include <cmath>

namespace
{
std::array<double, 3> rgb_to_hsv(const std::array<double, 3>& rgb)
{
  const double r = std::clamp(rgb[0], 0.0, 1.0);
  const double g = std::clamp(rgb[1], 0.0, 1.0);
  const double b = std::clamp(rgb[2], 0.0, 1.0);

  const double cmax = std::max(r, std::max(g, b));
  const double cmin = std::min(r, std::min(g, b));
  const double delta = cmax - cmin;

  double h;

  if (delta == 0.0) {
    h = 0.0;
  } else if (cmax == r) {
    h = std::fmod((g - b) / delta, 6.0);
  } else if (cmax == g) {
    h = ((b - r) / delta + 2.0);
  } else if (cmax == b) {
    h = ((r - g) / delta + 4.0);
  } else {
    Q_UNREACHABLE();
  }
  h *= M_PI / 3.0;
  if (h < 0.0) {
    h += 2 * M_PI;
  }

  const double s = cmax == 0.0 ? 0.0 : delta / cmax;
  const double v = cmax;
  h /= 2 * M_PI;

  return {h, s, v};
}

std::array<double, 3> hsv_to_rgb(const std::array<double, 3>& hsv)
{
  double h = hsv[0] * 2.0 * M_PI;
  h = std::fmod(h, 2 * M_PI);
  if (h < 0.0) {
    h += 2 * M_PI;
  }
  h = h * 3.0 / M_PI;
  const double c = hsv[2] * hsv[1];
  const double x = c * (1.0 - std::abs(std::fmod(h, 2.0) - 1));
  const double m = hsv[2] - c;
  switch (static_cast<int>(h)) {
  case 0:
    return {c + m, x + m, m};
  case 1:
    return {x + m, c + m, m};
  case 2:
    return {m, c + m, x + m};
  case 3:
    return {m, x + m, c + m};
  case 4:
    return {x + m, m, c + m};
  case 5:
    return {c + m, m, x + m};
  case 6:
  default:
    Q_UNREACHABLE();
    return {0.0, 0.0, 0.0};
  }
}

bool decode_hex(const QString& code, std::array<double, 4>& rgb)
{
  static const auto decode = [](const QString& code, int offset, double& v) {
    if (code.size() >= offset + 2) {
      bool ok;
      v = code.midRef(offset, 2).toInt(&ok, 16) / 255.0;
      return ok;
    } else {
      return false;
    }
  };
  if (code.at(0) != '#' || (code.size() != 7 && code.size() != 9)) {
    return false;
  } else {
    if (code.size() == 7 || code.size() == 9) {
      if (!decode(code, 1, rgb[0]) || !decode(code, 3, rgb[1]) || !decode(code, 5, rgb[2])) {
        return false;
      }
      if (code.size() == 9) {
        return decode(code, 7, rgb[3]);
      } else {
        rgb[3] = 1.0;
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
const std::map<Color::Model, std::array<QString, 4>> Color::component_names{
    {Color::Model::HSVA,
     {
         QT_TRANSLATE_NOOP("Color", "Hue"),
         QT_TRANSLATE_NOOP("Color", "Saturation"),
         QT_TRANSLATE_NOOP("Color", "Value"),
         QT_TRANSLATE_NOOP("Color", "Alpha"),
     }},
    {Color::Model::RGBA,
     {
         QT_TRANSLATE_NOOP("Color", "Red"),
         QT_TRANSLATE_NOOP("Color", "Green"),
         QT_TRANSLATE_NOOP("Color", "Blue"),
         QT_TRANSLATE_NOOP("Color", "Alpha"),
     }}};

Color::Color() : Color(Model::RGBA, {0.0, 0.0, 0.0, 1.0})
{
}

Color::Color(Color::Model model, const std::array<double, 3> components, double alpha)
    : Color(model, {components[0], components[1], components[2], alpha})
{
}

Color::Color(Color::Model model, const std::array<double, 4> components)
    : m_components(components), m_current_model(model)
{
}

Color::Color(const QString& name) : m_current_model(Model::Named), m_name(name)
{
}

Color::Color(const QColor& c) : Color(Color::RGBA, {c.redF(), c.greenF(), c.blueF(), c.alphaF()})
{
}

std::array<double, 4>
Color::convert(Color::Model from, Color::Model to, const std::array<double, 4> values)
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
  case Role::Red:
    [[fallthrough]];
  case Role::Green:
    [[fallthrough]];
  case Role::Blue:
    return Model::RGBA;
  case Role::Hue:
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
  static const auto to_hex = [](float f) {
    const int i = std::clamp(static_cast<int>(std::round(f * 255)), 0, 255);
    const QString str = QString("%1").arg(static_cast<int>(i), 2, 16, QChar('0'));
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
    if (!Application::instance().scene.named_colors().resolve(m_name, *this)) {
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
  case Role::Red:
    [[fallthrough]];
  case Role::Green:
    [[fallthrough]];
  case Role::Blue:
    convert(Model::RGBA);
    component(role) = value;
    return;
  case Role::Hue:
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
      return a.m_components < b.m_components;
    }
  }
}

std::ostream& operator<<(std::ostream& ostream, const Color& color)
{
  const QString id = [color]() {
    if (color.model() == Color::Model::Named) {
      return color.name();
    } else {
      QStringList cs;
      const auto components = color.components(color.model());
      for (std::size_t i = 0; i < components.size(); ++i) {
        const auto component_names = Color::component_names.at(color.model());
        cs.append(QString("%1: %2").arg(component_names[i]).arg(components[i]));
      }
      return cs.join(", ");
    }
  }();
  ostream << QString("Color[%1]").arg(id).toStdString();
  return ostream;
}

}  // namespace omm
