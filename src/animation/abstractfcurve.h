#pragma once

#include "aspects/serializable.h"
#include <map>
#include "abstractfactory.h"
#include "serializers/abstractserializer.h"

namespace omm
{

class AbstractPropertyOwner;
class Property;

class AbstractFCurve
    : public Serializable
    , public AbstractFactory<std::string, AbstractFCurve>
    , public ReferencePolisher
{
public:
  explicit AbstractFCurve() = default;
  static constexpr auto PROPERTY_KEY_KEY = "property";
  static constexpr auto OWNER_KEY = "owner";
  static constexpr auto KEY_VALUES_KEY = "keys";
  static constexpr auto TYPE_KEY = "type";

  void serialize(AbstractSerializer& serializer, const Pointer& pointer) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& pointer) override;
  virtual std::size_t count() const = 0;

  AbstractPropertyOwner* owner() const;
  const std::string property_key() const;
  void set_owner(AbstractPropertyOwner& owner, const std::string& property_key);

protected:
  struct FCurveSegment {
    int left_frame;
    double left_value;
    int left_offset;
    double left_tangent;

    int right_frame;
    double right_valye;
    int right_offset;
    double right_tangent;
  };

  double interpolate(int frame, const FCurveSegment& segnment) const;
  virtual void serialize_keyvalue(AbstractSerializer&, std::size_t index, const Pointer& pointer) const = 0;
  virtual void deserialize_keyvalue(AbstractDeserializer&, std::size_t index, const Pointer& pointer) = 0;

  void update_referenes(const std::map<std::size_t, AbstractPropertyOwner *> &map) override
  {
    m_owner = map.at(m_owner_id);
  }


private:
  AbstractPropertyOwner* m_owner = nullptr;
  std::string m_property_key = "";

  // this field is only required temporarily during deserialization
  std::size_t m_owner_id;
};

}  // namespace omm
