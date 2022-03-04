#include "mainwindow/exportoptions.h"
#include "aspects/abstractpropertyowner.h"
#include "objects/view.h"
#include "serializers/abstractdeserializer.h"

namespace omm
{

class ExportOptions::ReferencePolisher : public omm::serialization::ReferencePolisher
{
public:
  explicit ReferencePolisher(const std::size_t view_id, ExportOptions& export_options)
    : m_view_id(view_id)
    , m_export_options(export_options)
  {

  }

private:
  void update_references(const std::map<std::size_t, AbstractPropertyOwner*>& map) override
  {
    if (m_view_id == 0) {
      m_export_options.view = nullptr;
    } else {
      const auto it = map.find(m_view_id);
      if (it == map.end()) {
        LWARNING << "Failed to restore reference to view " << m_view_id;
        m_export_options.view = nullptr;
      } else if (it->second->type() == View::TYPE) {
        m_export_options.view = type_cast<View*>(it->second);
      } else {
        LWARNING << "Reference " << m_view_id << " points to an object of type other than View.";
      }
    }
  }

  std::size_t m_view_id;
  ExportOptions& m_export_options;
};

void ExportOptions::serialize(serialization::SerializerWorker& worker) const
{
  worker.sub(VIEW_KEY)->set_value(view);
  worker.sub(X_RESOLUTION_KEY)->set_value(x_resolution);
  worker.sub(START_FRAME_KEY)->set_value(start_frame);
  worker.sub(END_FRAME_KEY)->set_value(end_frame);
  worker.sub(PATTERN_KEY)->set_value(pattern);
  worker.sub(ANIMATED_KEY)->set_value(animated);
  worker.sub(SCALE_KEY)->set_value(scale);
  worker.sub(FORMAT_KEY)->set_value(static_cast<int>(format));
}

void ExportOptions::deserialize(serialization::DeserializerWorker& worker)
{
  const auto view_id = worker.sub(VIEW_KEY)->get_size_t();
  x_resolution = worker.sub(X_RESOLUTION_KEY)->get_int();
  start_frame = worker.sub(START_FRAME_KEY)->get_int();
  end_frame = worker.sub(END_FRAME_KEY)->get_int();
  pattern = worker.sub(PATTERN_KEY)->get_string();
  animated = worker.sub(ANIMATED_KEY)->get_bool();
  scale = worker.sub(SCALE_KEY)->get_double();
  format = static_cast<Format>(worker.sub(FORMAT_KEY)->get_int());
  worker.deserializer().register_reference_polisher(std::make_unique<ReferencePolisher>(view_id, *this));
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
