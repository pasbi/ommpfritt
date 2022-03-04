#pragma once

#include "common.h"
#include "variant.h"
#include <QCoreApplication>
#include <QObject>
#include <map>

namespace omm
{

class Property;
struct Knot;

namespace serialization
{
class SerializerWorker;
class DeserializerWorker;
}  // namespace serialization

/**
 * @brief The AbstractTrack class is the base class for all track.
 *  It is required to store the association to a (owner, property_key)-pair rather than to a
 *  property directly because properties do not have serialization-persistent ids.
 *  It would be impossible to restore an track at deserialization since the property it belongs
 *  to is not known at that time.
 */
class Track
{
  Q_DECLARE_TR_FUNCTIONS(Track)
public:
  enum class Interpolation { Step, Linear, Bezier };
  static QString interpolation_label(Interpolation interpolation);

  explicit Track(Property& property);
  Track(Track&&) = delete;
  Track(const Track&) = delete;
  Track& operator=(Track&&) = delete;
  Track& operator=(const Track&) = delete;
  ~Track();
  [[nodiscard]] std::unique_ptr<Track> clone() const;
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

  void serialize(serialization::SerializerWorker& worker) const;
  void deserialize(serialization::DeserializerWorker& worker);

  [[nodiscard]] bool has_keyframe(int frame) const
  {
    return m_knots.find(frame) != m_knots.end();
  }

  [[nodiscard]] double interpolate(double frame, std::size_t channel) const;
  [[nodiscard]] variant_type interpolate(double frame) const;
  [[nodiscard]] Knot& knot(int frame) const;

  [[nodiscard]] std::vector<int> key_frames() const;
  void apply(int frame) const;
  void move_knot(int old_frame, int new_frame);

  /**
   * @brief record creates a new key-value, overwriting existing keys.
   * @param frame the frame
   * @param value the value's type must be the same as the properties' type.
   * @note this function must only be called from the @code Animator instance.
   * @see Animator::set_key
   */
  void insert_knot(int frame, std::unique_ptr<Knot> knot);

  /**
   * @brief remove_knot removes the key at given frame. Does nothing if there is no such
   *  key.
   * @param frame the frame
   * @note this function must only be called from the @code Animator instance.
   * @see Animator::remove_key
   */
  std::unique_ptr<Knot> remove_knot(int frame);

  [[nodiscard]] QString type() const;
  [[nodiscard]] Property& property() const
  {
    return m_property;
  }

  /**
   * @brief is_consistent returns whether the key value of the track at @code frame matches the
   * value of the property. Never interpolates the value.
   * @note if there is no key frame at @code frame, then true is returned.
   */
  [[nodiscard]] bool is_consistent(int frame) const;

  [[nodiscard]] bool is_numerical() const;

  void set_interpolation(Interpolation interpolation);
  [[nodiscard]] Interpolation interpolation() const;

private:
  Property& m_property;
  std::map<int, std::unique_ptr<Knot>> m_knots;
  Interpolation m_interpolation = Interpolation::Linear;
};

}  // namespace omm
