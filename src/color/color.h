#pragma once

#include <QColor>
#include <array>
#include <iterator>
#include <map>
#include <vector>

namespace omm
{

namespace serialization
{
class SerializerWorker;
class DeserializerWorker;
}  // namespace serialization

/**
 * @brief The Color class r, g, b, h, s, v are in range [0, 1]
 */
class Color
{
private:
  std::array<double, 4> m_components;

public:
  enum Model { RGBA, HSVA, Named };
  Color(Model model, const std::array<double, 3>& components, double alpha);
  Color(Model model, const std::array<double, 4>& components);
  explicit Color(const QString& name);
  explicit Color(const QColor& c);
  explicit Color();
  [[nodiscard]] static QString component_name(const Model& model, std::size_t component);

  [[nodiscard]] QString to_html() const;
  [[nodiscard]] static Color from_html(const QString& html, bool* ok = nullptr);
  [[nodiscard]] static Color from_qcolor(const QColor& color);
  [[nodiscard]] QColor to_qcolor() const;

  void to_ordinary_color();
  [[nodiscard]] QString name() const;

  enum class Role { Red, Green, Blue, Hue, Saturation, Value, Alpha };

  void set(Role role, double value);
  [[nodiscard]] double get(Role role) const;

  static std::array<double, 4> convert(Model from, Model to, const std::array<double, 4>& values);
  void convert(Model to);
  [[nodiscard]] Color convert(Model to) const;

  [[nodiscard]] std::array<double, 4> components(Model model) const;
  void set_components(Model model, const std::array<double, 4>& components);
  [[nodiscard]] Model model() const
  {
    return m_current_model;
  }

  [[nodiscard]] QString to_string() const;
  void serialize(serialization::SerializerWorker& worker) const;
  void deserialize(serialization::DeserializerWorker& worker);

private:
  Model m_current_model;
  QString m_name = "";

  friend bool operator==(const Color& a, const Color& b);
  friend bool operator<(const Color& a, const Color& b);

  /**
   * @brief component returns a reference to the component represented by a certain role.
   * @param role the role. Note that the role must belong to the underlying model.
   * @return a reference to the component.
   */
  double& component(Role role);

  static Model model(Role role, Model tie);
};

namespace Colors
{
static const Color RED(Color::Model::RGBA, {1.0, 0.0, 0.0, 1.0});
static const Color GREEN(Color::Model::RGBA, {0.0, 1.0, 0.0, 1.0});
static const Color BLUE(Color::Model::RGBA, {0.0, 0.0, 1.0, 1.0});
static const Color YELLOW(Color::Model::RGBA, {1.0, 1.0, 0.0, 1.0});
static const Color BLACK(Color::Model::RGBA, {0.0, 0.0, 0.0, 1.0});
static const Color WHITE(Color::Model::RGBA, {1.0, 1.0, 1.0, 1.0});
static const Color CERULEAN(Color::Model::RGBA, {0.1, 0.52, 0.82, 1.0});

}  // namespace Colors

[[nodiscard]] bool operator==(const Color& a, const Color& b);
[[nodiscard]] bool operator!=(const Color& a, const Color& b);
[[nodiscard]] bool operator<(const Color& a, const Color& b);

}  // namespace omm
