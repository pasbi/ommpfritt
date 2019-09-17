//#pragma once

//#include "animation/track.h"
//#include "properties/floatproperty.h"

//namespace omm
//{

//class FloatTrack : public Track<FloatProperty>
//{
//public:
//  static constexpr auto TYPE = "FloatTrack";
//  std::string type() const override { return TYPE; }
//  double interpolate(int frame) const override;
//  std::size_t count() const override;

//  struct KeyValue
//  {
//    KeyValue(double value);

//    double value;

//    int left_offset;
//    double left_value;
//    int right_offset;
//    double right_value;
//  };

//  bool has_key_at(int frame) const override;
//  void record(int frame, Property& property) override;
//  void remove_key_at(int frame) override;

//protected:
//  void deserialize_keyvalue(AbstractDeserializer &, std::size_t i, const Pointer &pointer) override;
//  void serialize_keyvalue(AbstractSerializer &, std::size_t i, const Pointer &pointer) const override;

//private:
//  std::map<int, KeyValue> m_keyvalues;
//};

//}  // namespace omm
