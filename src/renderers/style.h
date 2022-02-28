#pragma once

#include "aspects/propertyowner.h"
#include "nodesystem/nodesowner.h"
#include <QIcon>
#include <memory>

namespace omm
{

class MarkerProperties;
class OffscreenRenderer;
class Scene;
struct PainterOptions;
struct Texture;

class Style
    : public PropertyOwner<Kind::Style>
    , public nodes::NodesOwner
{
  Q_OBJECT
public:
  explicit Style(Scene* scene = nullptr);
  ~Style() override;
  Style(const Style& other);
  Style(Style&&) = delete;
  Style& operator=(const Style&) = delete;
  Style& operator=(Style&&) = delete;
  QString type() const override;
  static constexpr auto TYPE = "Style";
  std::unique_ptr<Style> clone() const;  // provided for interface consistency
  Flag flags() const override;
  Texture render_texture(const Object& object,
                         const QSize& size,
                         const QRectF& roi,
                         const PainterOptions& options) const;

  void serialize(serialization::SerializerWorker& worker) const override;
  void deserialize(serialization::DeserializerWorker& worker) override;

public:
  static constexpr auto PEN_IS_ACTIVE_KEY = "pen/active";
  static constexpr auto PEN_COLOR_KEY = "pen/color";
  static constexpr auto PEN_WIDTH_KEY = "pen/width";
  static constexpr auto STROKE_STYLE_KEY = "pen/stroke";
  static constexpr auto JOIN_STYLE_KEY = "pen/join";
  static constexpr auto CAP_STYLE_KEY = "pen/cap";
  static constexpr auto COSMETIC_KEY = "pen/cosmetic";
  static constexpr auto START_MARKER = "pen/start-marker";
  static constexpr auto END_MARKER = "pen/end-marker";
  static constexpr auto BRUSH_IS_ACTIVE_KEY = "brush/active";
  static constexpr auto BRUSH_COLOR_KEY = "brush/color";
  static constexpr auto EDIT_NODES_PROPERTY_KEY = "edit-nodes";

  static constexpr auto NODES_POINTER = "nodes";

  const std::unique_ptr<MarkerProperties> start_marker;
  const std::unique_ptr<MarkerProperties> end_marker;
  void on_property_value_changed(Property* property) override;

private:
  std::unique_ptr<OffscreenRenderer> m_offscreen_renderer;
  void update_uniform_values() const;
  std::set<Property*> m_uniform_values;
  void polish();

private:
  void set_code(const QString& code) const;
  void set_error(const QString& error) const;
};

}  // namespace omm
