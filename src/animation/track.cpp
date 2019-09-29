#include "animation/track.h"
#include "logging.h"
#include "common.h"
#include "properties/property.h"
#include "scene/scene.h"

namespace
{

using Interpolation = omm::Track::Interpolation;

template<typename T> struct Segment
{
  std::array<T, 4> values;
  std::array<double, 4> frames;

  template<typename S, typename F> Segment<S> convert(F&& f) const
  {
    Segment<S> s;
    std::transform(values.begin(), values.end(), s.values.begin(), f);
    return s;
  }
};

template<typename T> T step_interpolate(const Segment<T>& segment, double t)
{
  if (t == 1.0) {
    return segment.values.back();
  } else {
    return segment.values.front();
  }
}

bool interpolate(const Segment<bool>& segment, double t, Interpolation)
{
  return step_interpolate(segment, t);
}

double interpolate(const Segment<double>& segment, double t, Interpolation interpolation)
{
  switch (interpolation) {
  case Interpolation::Step:
    return step_interpolate(segment, t);
  case Interpolation::Linear:
    return (1.0-t) * segment.values[0] + t * segment.values[3];
  case Interpolation::Bezier:
    return 0.0;
  default:
    Q_UNREACHABLE();
    return 0.0;
  }
}

omm::Color interpolate(const Segment<omm::Color>& segment, double t, Interpolation interpolation)
{
  using Hsva = std::array<double, 4>;
  const Segment<Hsva> hsva_segment = segment.convert<Hsva>([](const omm::Color& c) {
    Hsva hsva;
    c.to_hsv(hsva[0], hsva[1], hsva[2]);
    hsva[3] = c.alpha();
    return hsva;
  });

  Hsva interpolated;
  for (std::size_t i = 0; i < 4; ++i) {
    const Segment<double> segd = hsva_segment.convert<double>([i](const Hsva& hsva) {
      return hsva[i];
    });
    interpolated[i] = interpolate(segd, t, interpolation);
  }
  return omm::Color::from_hsv(interpolated[0], interpolated[1], interpolated[2], interpolated[3]);
}

int interpolate(const Segment<int>& segment, double t, Interpolation interpolation)
{
  const Segment<double> dseg = segment.convert<double>([](int i) {
    return static_cast<double>(i);
  });

  return static_cast<int>(interpolate(dseg, t, interpolation));
}

omm::AbstractPropertyOwner* interpolate(const Segment<omm::AbstractPropertyOwner*>& segment,
                                        double t, Interpolation)
{
  return step_interpolate(segment, t);
}

std::string interpolate(const Segment<std::string>& segment, double t, Interpolation)
{
  return step_interpolate(segment, t);
}

std::size_t interpolate(const Segment<std::size_t>& segment, double t, Interpolation)
{
  return step_interpolate(segment, t);
}

omm::TriggerPropertyDummyValueType
interpolate(const Segment<omm::TriggerPropertyDummyValueType>&, double, Interpolation)
{
  return omm::TriggerPropertyDummyValueType();
}

template<typename T>
omm::Vec2<T> interpolate(const Segment<omm::Vec2<T>>& segment, double t, Interpolation interpolation)
{
  const Segment<T> xs = segment.template convert<T>([](const omm::Vec2<T>& v) { return v[0]; });
  const Segment<T> ys = segment.template convert<T>([](const omm::Vec2<T>& v) { return v[1]; });
  return omm::Vec2<T>(interpolate(xs, t, interpolation), interpolate(ys, t, interpolation));
}

}

namespace omm
{

Track::Track(Property &property) : m_property(property)
{
}

void Track::serialize(AbstractSerializer& serializer, const Pointer& pointer) const
{
  serializer.set_value(type(), make_pointer(pointer, TYPE_KEY));
  const auto knots_pointer = make_pointer(pointer, KNOTS_KEY);
  const auto key_frames = this->key_frames();
  serializer.start_array(key_frames.size(), knots_pointer);
  for (std::size_t i = 0; i < key_frames.size(); ++i) {
    const int frame = key_frames.at(i);
    const Knot& knot = m_knots.at(frame);
    const auto knot_pointer = make_pointer(knots_pointer, i);
    serializer.set_value(frame, make_pointer(knot_pointer, FRAME_KEY));
    serializer.set_value(knot.value, make_pointer(knot_pointer, VALUE_KEY));
    serializer.set_value(knot.left_offset, make_pointer(knot_pointer, LEFT_OFFSET_KEY));
    serializer.set_value(knot.left_value, make_pointer(knot_pointer, LEFT_VALUE_KEY));
    serializer.set_value(knot.right_offset, make_pointer(knot_pointer, RIGHT_OFFSET_KEY));
    serializer.set_value(knot.right_value, make_pointer(knot_pointer, RIGHT_VALUE_KEY));
  }
  serializer.set_value(m_interpolation, make_pointer(pointer, INTERPOLATION_KEY));
  serializer.end_array();
}

void Track::deserialize(AbstractDeserializer& deserializer, const Pointer& pointer)
{
  const std::string type = deserializer.get_string(make_pointer(pointer, TYPE_KEY));
  m_interpolation = deserializer.get<Interpolation>(make_pointer(pointer, INTERPOLATION_KEY));

  const auto knots_pointer = make_pointer(pointer, KNOTS_KEY);
  const std::size_t n = deserializer.array_size(knots_pointer);
  for (std::size_t i = 0; i < n; ++i) {
    const auto knot_pointer = make_pointer(knots_pointer, i);
    Knot knot(deserializer.get(make_pointer(knot_pointer, VALUE_KEY), type));
    knot.left_offset = deserializer.get_int(make_pointer(knot_pointer, LEFT_OFFSET_KEY));
    knot.left_value = deserializer.get(make_pointer(knot_pointer, LEFT_VALUE_KEY), type);
    knot.right_offset = deserializer.get_int(make_pointer(knot_pointer, RIGHT_OFFSET_KEY));
    knot.right_value = deserializer.get(make_pointer(knot_pointer, RIGHT_VALUE_KEY), type);
    const int frame = deserializer.get_int(make_pointer(knot_pointer, FRAME_KEY));
    m_knots.insert(std::pair(frame, knot));
  }
}

Track::Knot Track::remove_knot(int frame)
{
  assert (m_knots.find(frame) != m_knots.end());
  return m_knots.extract(frame).mapped();
}

variant_type Track::interpolate(double frame) const
{
  assert(!m_knots.empty());

  if (const auto it = m_knots.find(frame); it != m_knots.end()) {
    return it->second.value;
  }

  const Knot* left = nullptr;
  int left_frame;
  const Knot* right = nullptr;
  int right_frame;
  for (auto it = m_knots.cbegin(); it != m_knots.cend(); ++it) {
    if (it->first <= frame) {
      left_frame = it->first;
      left = &it->second;
    } else if (it->first > frame) {
      right_frame = it->first;
      right = &it->second;
      break;
    }
  }

  if (left == nullptr && right != nullptr) {
    return right->value;
  } else if (left != nullptr && right == nullptr) {
    return left->value;
  } else {
    assert(left != nullptr && right != nullptr);
    return std::visit([=](auto&& arg) -> variant_type {
      using T = std::decay_t<decltype(arg)>; 

      const double span = right_frame - left_frame;
      const double left_t = left->right_offset / span;
      const double right_t = (right_frame + right->left_offset - left_frame) / span;

      const Segment<T> segment {
         { std::get<T>(left->value),       std::get<T>(left->right_value),
           std::get<T>(right->left_value), std::get<T>(right->value) },
          { 0.0, left_t, right_t, 1.0 }
      };

      const double t = (frame - left_frame) / span;
      return ::interpolate(segment, t, m_interpolation);
    }, right->value);
  }
}

Track::Knot Track::interpolate_knot(double frame) const
{
  // TODO we need something clever here for Bezier interpolation
  const variant_type value = interpolate(frame);
  return Knot(value);
}

Track::Knot Track::knot_at(double frame) const
{
  if (const auto it = m_knots.find(frame); it != m_knots.end()) {
    return it->second;
  } else {
    return interpolate_knot(frame);
  }
}

std::vector<int> Track::key_frames() const
{
  return ::transform<int, std::vector>(m_knots, [](const auto& p) {
    return p.first;
  });
}

void Track::apply(int frame) const
{
  if (!m_knots.empty()) {
    property().set(interpolate(frame));
  }
}

void Track::move_knot(int old_frame, int new_frame)
{
  m_knots.insert(std::pair(new_frame, m_knots.extract(old_frame).mapped()));
}

void Track::insert_knot(int frame, const Knot &knot)
{
  assert(knot.value.index() == property().variant_value().index());
  const auto it = m_knots.find(frame);
  if (it == m_knots.end() || it->second != knot) {
    m_knots.insert(std::pair(frame, knot));
  }
}

std::string Track::type() const
{
  static const std::string property_suffix = "Property";
  const std::string type = property().type();
  assert(type.size() > property_suffix.size());
  return type.substr(0, type.size() - property_suffix.size());
}

bool Track::is_consistent(int frame) const
{
  if (const auto it = m_knots.find(frame); it != m_knots.end()) {
    return it->second.value == property().variant_value();
  } else {
    return true;
  }
}

bool Track::is_numerical() const
{
  return property().is_numerical();
}

Track::Knot::Knot(const variant_type &value)
  : value(value), left_offset(0), left_value(value), right_offset(0), right_value(value)
{
}

bool Track::Knot::operator==(const Track::Knot& other) const
{
  return value == other.value
      && left_value == other.left_value
      && right_value == other.right_value
      && left_offset == other.left_offset
      && right_offset == other.right_offset;
}

bool Track::Knot::operator!=(const Track::Knot& other) const
{
  return !(*this == other);
}

}  // namespace omm
