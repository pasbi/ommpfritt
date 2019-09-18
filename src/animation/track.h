#pragma once

#include "aspects/serializable.h"
#include <map>
#include "abstractfactory.h"
#include <QObject>
#include "serializers/abstractserializer.h"
#include "common.h"
#include "properties/property.h"

namespace omm
{

class AbstractPropertyOwner;

/**
 * @brief The AbstractTrack class is the base class for all track.
 *  It is required to store the association to a (owner, property_key)-pair rather than to a
 *  property directly because properties do not have serialization-persistent ids.
 *  It would be impossible to restore an track at deserialization since the property it belongs
 *  to is not known at that time.
 */
class Track
  : public QObject
  , public Serializable
  , public ReferencePolisher
{
  Q_OBJECT
public:
  struct Knot {
    Knot(const variant_type& value);
    variant_type value;
    int left_offset;
    variant_type left_value;
    int right_offset;
    variant_type right_value;
  };

  enum class Interpolation { Step, Linear, Bezier };

  explicit Track() = default;
  static constexpr auto PROPERTY_KEY_KEY = "property";
  static constexpr auto OWNER_KEY = "owner";
  static constexpr auto KNOTS_KEY = "knots";
  static constexpr auto TYPE_KEY = "type";
  static constexpr auto FRAME_KEY = "frame";
  static constexpr auto VALUE_KEY = "value";
  static constexpr auto LEFT_OFFSET_KEY = "left-offset";
  static constexpr auto LEFT_VALUE_KEY = "left-value";
  static constexpr auto RIGHT_OFFSET_KEY = "right-offset";
  static constexpr auto RIGHT_VALUE_KEY = "right-value";
  static constexpr auto INTERPOLATION_KEY = "interpolation";

  void serialize(AbstractSerializer& serializer, const Pointer& pointer) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& pointer) override;

  AbstractPropertyOwner* owner() const { return m_owner; }
  const std::string property_key() const { return m_property_key; }
  void set_owner(AbstractPropertyOwner& owner, const std::string& property_key);
  bool has_keyframe(int frame) const { return m_knots.find(frame) != m_knots.end(); }
  void record(int frame, Property& property);
  void remove_keyframe(int frame);
  variant_type interpolate(double frame) const;
  const Knot& knot_at(int frame) const;
  Knot& knot_at(int frame);
  std::vector<int> key_frames() const;
  void apply(int frame) const;

  std::string type() const;
  Property& property() const;

  void update_referenes(const std::map<std::size_t, AbstractPropertyOwner *> &map) override
  {
    m_owner = map.at(m_owner_id);
  }

Q_SIGNALS:
  void track_changed();


private:
  AbstractPropertyOwner* m_owner = nullptr;
  std::string m_property_key = "";

  // this field is only required temporarily during deserialization
  std::size_t m_owner_id;

  std::map<int, Knot> m_knots;
  Interpolation m_interpolation = Interpolation::Linear;
};

}  // namespace omm
