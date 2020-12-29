#pragma once

#include "aspects/serializable.h"
#include "serializers/abstractserializer.h"

namespace omm
{
class View;

struct ExportOptions
    : Serializable
    , ReferencePolisher {
  bool operator==(const ExportOptions& other) const;
  bool operator!=(const ExportOptions& other) const;
  void serialize(AbstractSerializer& serializer, const Pointer& pointer) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& pointer) override;

  enum class Format {Raster, Vector};

  static constexpr auto VIEW_KEY = "view";
  static constexpr auto X_RESOLUTION_KEY = "x_resolution";
  static constexpr auto PATTERN_KEY = "pattern";
  static constexpr auto START_FRAME_KEY = "start";
  static constexpr auto END_FRAME_KEY = "end";
  static constexpr auto ANIMATED_KEY = "animated";
  static constexpr auto FORMAT_KEY = "format";
  static constexpr auto SCALE_KEY = "scale";

  View* view = nullptr;
  int x_resolution;
  QString pattern;
  int start_frame;
  int end_frame;
  bool animated;
  Format format;
  double scale;

private:
  std::size_t m_view_id;  // required for deserialization only

protected:
  void update_references(const std::map<std::size_t, AbstractPropertyOwner*>& map) override;
};

}  // namespace omm
