#pragma once

#include <QString>

namespace omm
{

class View;

namespace serialization
{
class SerializerWorker;
class DeserializerWorker;
}  // namespace serialization

struct ExportOptions
{
  bool operator==(const ExportOptions& other) const;
  bool operator!=(const ExportOptions& other) const;
  void serialize(serialization::SerializerWorker& worker) const;
  void deserialize(serialization::DeserializerWorker& worker);

  enum class Format { Raster, Vector };

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
  class ReferencePolisher;
};

}  // namespace omm
