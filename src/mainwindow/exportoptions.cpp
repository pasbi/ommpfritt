#include "mainwindow/exportoptions.h"
#include "aspects/abstractpropertyowner.h"
#include "objects/view.h"
#include "serializers/abstractserializer.h"

namespace omm
{
void ExportOptions::serialize(AbstractSerializer& serializer, const Pointer& pointer) const
{
  Serializable::serialize(serializer, pointer);
  serializer.set_value(view, make_pointer(pointer, VIEW_KEY));
  serializer.set_value(x_resolution, make_pointer(pointer, X_RESOLUTION_KEY));
  serializer.set_value(start_frame, make_pointer(pointer, START_FRAME_KEY));
  serializer.set_value(end_frame, make_pointer(pointer, END_FRAME_KEY));
  serializer.set_value(pattern, make_pointer(pointer, PATTERN_KEY));
  serializer.set_value(animated, make_pointer(pointer, ANIMATED_KEY));
  serializer.set_value(static_cast<int>(format), make_pointer(pointer, FORMAT_KEY));
}

void ExportOptions::deserialize(AbstractDeserializer& deserializer, const Pointer& pointer)
{
  Serializable::deserialize(deserializer, pointer);
  m_view_id = deserializer.get_size_t(make_pointer(pointer, VIEW_KEY));
  x_resolution = deserializer.get_int(make_pointer(pointer, X_RESOLUTION_KEY));
  start_frame = deserializer.get_int(make_pointer(pointer, START_FRAME_KEY));
  end_frame = deserializer.get_int(make_pointer(pointer, END_FRAME_KEY));
  pattern = deserializer.get_string(make_pointer(pointer, PATTERN_KEY));
  animated = deserializer.get_int(make_pointer(pointer, ANIMATED_KEY));
  format = static_cast<Format>(deserializer.get_int(make_pointer(pointer, FORMAT_KEY)));
  deserializer.register_reference_polisher(*this);
}

void ExportOptions::update_references(const std::map<std::size_t, AbstractPropertyOwner*>& map)
{
  if (m_view_id == 0) {
    view = nullptr;
  } else {
    const auto it = map.find(m_view_id);
    if (it == map.end()) {
      LWARNING << "Failed to restore reference to view " << m_view_id;
      view = nullptr;
    } else if (it->second->type() == View::TYPE) {
      view = type_cast<View*>(it->second);
    } else {
      LWARNING << "Reference " << m_view_id << " points to an object of type other than View.";
    }
  }
}

bool ExportOptions::operator==(const ExportOptions& other) const
{
  static const auto to_tuple = [](const ExportOptions& o) {
    return std::tuple{o.view,
                      o.x_resolution,
                      o.pattern,
                      o.start_frame,
                      o.end_frame,
                      o.animated,
                      o.format};
  };
  return to_tuple(*this) == to_tuple(other);
}

bool ExportOptions::operator!=(const ExportOptions& other) const
{
  return !(*this == other);
}

}  // namespace omm
