#pragma once

#include <QIcon>
#include "aspects/propertyowner.h"
#include "color/color.h"
#include "properties/propertygroups/markerproperties.h"

namespace omm
{

class Scene;

class Style
  : public PropertyOwner<AbstractPropertyOwner::Kind::Style>
  , public virtual Serializable
{
public:
  explicit Style(Scene* scene = nullptr);
  Style(const Style& other);
  std::string type() const;
  static constexpr auto TYPE = "Style";
  std::unique_ptr<Style> clone() const;  // provided for interface consistency
  QIcon icon() const;
  Flag flags() const override;
  Scene* scene() const;

private:
  Scene* const m_scene;

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

  const MarkerProperties start_marker;
  const MarkerProperties end_marker;
};

class SolidStyle : public Style
{
public:
  SolidStyle(const Color& color, Scene* scene = nullptr);
};

class ContourStyle : public Style
{
public:
  ContourStyle(const Color& color, Scene* scene = nullptr);
  ContourStyle(const Color& color, double width, Scene* scene = nullptr);
};

}  // namespace omm
