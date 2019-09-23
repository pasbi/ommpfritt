#pragma once

#include "aspects/serializable.h"
#include <map>
#include "abstractfactory.h"
#include <QObject>
#include "serializers/abstractserializer.h"
#include "common.h"

namespace omm
{

class Property;

/**
 * @brief The AbstractTrack class is the base class for all track.
 *  It is required to store the association to a (owner, property_key)-pair rather than to a
 *  property directly because properties do not have serialization-persistent ids.
 *  It would be impossible to restore an track at deserialization since the property it belongs
 *  to is not known at that time.
 */
class Track : public QObject, public Serializable
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

  explicit Track(Property& property);
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

  bool has_keyframe(int frame) const { return m_knots.find(frame) != m_knots.end(); }
  variant_type interpolate(double frame) const;
  const Knot& knot_at(int frame) const;
  Knot& knot_at(int frame);
  std::vector<int> key_frames() const;
  void apply(int frame) const;
  void move_key(int old_frame, int new_frame);

  /**
   * @brief record creates a new key-value, overwriting existing keys.
   * @param frame the frame
   * @param value the value's type must be the same as the properties' type.
   * @note this function must only be called from the @code Animator instance.
   * @see Animator::set_key
   */
  void record(int frame, const variant_type& value);

  /**
   * @brief remove_keyframe removes the key at given frame. Does nothing if there is no such
   *  key.
   * @param frame the frame
   * @note this function must only be called from the @code Animator instance.
   * @see Animator::remove_key
   */
  void remove_keyframe(int frame);

  std::string type() const;
  Property& property() const { return m_property; }

  /**
   * @brief is_consistent returns whether the key value of the track at @code frame matches the
   * value of the property. Never interpolates the value.
   * @note if there is no key frame at @code frame, then true is returned.
   */
  bool is_consistent(int frame) const;

private:
  Property& m_property;
  std::map<int, Knot> m_knots;
  Interpolation m_interpolation = Interpolation::Linear;
};

}  // namespace omm
