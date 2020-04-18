#include "animation/track.h"
#include "logging.h"
#include "common.h"
#include "properties/property.h"
#include "scene/scene.h"

namespace
{

using Interpolation = omm::Track::Interpolation;

double interpolate(const std::array<double, 4>& segment, double t, Interpolation interpolation)
{
  std::array<double, 4> bernstein4 {
    1.0 * (1-t) * (1-t) * (1-t),
    3.0 *   t   * (1-t) * (1-t),
    3.0 *   t   *   t   * (1-t),
    1.0 *   t   *   t   *   t
  };
  switch (interpolation) {
  case Interpolation::Step:
    return segment[0];
  case Interpolation::Linear:
    return (1.0-t) * segment[0] + t * segment[3];
  case Interpolation::Bezier:
    return bernstein4[0] * segment[0]
         + bernstein4[1] * segment[1]
         + bernstein4[2] * segment[2]
         + bernstein4[3] * segment[3];
  default:
    Q_UNREACHABLE();
    return 0.0;
  }
}

}  // namespace

namespace omm
{

QString Track::interpolation_label(Track::Interpolation interpolation)
{
  switch (interpolation) {
  case Track::Interpolation::Step:
    return tr("Step");
  case Track::Interpolation::Bezier:
    return tr("Bezier");
  case Track::Interpolation::Linear:
    return tr("Linear");
  default:
    Q_UNREACHABLE();
    return "";
  }
}

Track::Track(Property &property) : m_property(property)
{
}

Track::~Track()
{
}

std::unique_ptr<Track> Track::clone() const
{
  auto track = std::make_unique<Track>(m_property);
  for (auto&& [frame, knot] : m_knots) {
    track->m_knots.insert({ frame, knot->clone() });
  }
  track->m_interpolation = m_interpolation;
  return track;
}

void Track::serialize(AbstractSerializer& serializer, const Pointer& pointer) const
{
  serializer.set_value(type(), make_pointer(pointer, TYPE_KEY));
  const auto knots_pointer = make_pointer(pointer, KNOTS_KEY);
  const auto key_frames = this->key_frames();
  serializer.start_array(key_frames.size(), knots_pointer);
  for (std::size_t i = 0; i < key_frames.size(); ++i) {
    const int frame = key_frames.at(i);
    const Knot& knot = *m_knots.at(frame);
    const auto knot_pointer = make_pointer(knots_pointer, i);
    serializer.set_value(frame, make_pointer(knot_pointer, FRAME_KEY));
    serializer.set_value(knot.value, make_pointer(knot_pointer, VALUE_KEY));
    if (is_numerical()) {
      serializer.set_value(knot.left_offset, make_pointer(knot_pointer, LEFT_VALUE_KEY));
      serializer.set_value(knot.right_offset, make_pointer(knot_pointer, RIGHT_VALUE_KEY));
    }
  }
  serializer.set_value(m_interpolation, make_pointer(pointer, INTERPOLATION_KEY));
  serializer.end_array();
}

void Track::deserialize(AbstractDeserializer& deserializer, const Pointer& pointer)
{
  const QString type = deserializer.get_string(make_pointer(pointer, TYPE_KEY));
  m_interpolation = deserializer.get<Interpolation>(make_pointer(pointer, INTERPOLATION_KEY));

  const auto knots_pointer = make_pointer(pointer, KNOTS_KEY);
  const std::size_t n = deserializer.array_size(knots_pointer);
  for (std::size_t i = 0; i < n; ++i) {
    const auto knot_pointer = make_pointer(knots_pointer, i);
    auto knot = std::make_unique<Knot>(deserializer, make_pointer(knot_pointer, VALUE_KEY), type);
    if (is_numerical()) {
      knot->left_offset = deserializer.get(make_pointer(knot_pointer, LEFT_VALUE_KEY), type);
      knot->right_offset = deserializer.get(make_pointer(knot_pointer, RIGHT_VALUE_KEY), type);
    }
    const int frame = deserializer.get_int(make_pointer(knot_pointer, FRAME_KEY));
    m_knots.insert(std::pair(frame, std::move(knot)));
  }
}

std::unique_ptr<Track::Knot> Track::remove_knot(int frame)
{
  assert (m_knots.find(frame) != m_knots.end());
  return std::move(m_knots.extract(frame).mapped());
}

double Track::interpolate(double frame, std::size_t channel) const
{
  // this can be optimized!
  return get_channel_value(interpolate(frame), channel);
}

variant_type Track::interpolate(double frame) const
{
  assert(!m_knots.empty());

  if (const auto it = m_knots.find(frame); it != m_knots.end()) {
    return it->second->value;
  }

  const Knot* left = nullptr;
  int left_frame;
  const Knot* right = nullptr;
  int right_frame;
  for (auto it = m_knots.cbegin(); it != m_knots.cend(); ++it) {
    if (it->first <= frame) {
      left_frame = it->first;
      left = it->second.get();
    } else if (it->first > frame) {
      right_frame = it->first;
      right = it->second.get();
      break;
    }
  }

  if (left == nullptr && right != nullptr) {
    return right->value;
  } else if (left != nullptr && right == nullptr) {
    return left->value;
  } else {
    assert(left != nullptr && right != nullptr);
    const std::size_t n = n_channels(left->value);
    assert(n == n_channels(right->value));
    if (n == 0) {
      return left->value;  // non-numerical types cannot be interpolated.
    } else {
      const double t = (frame - left_frame) / static_cast<double>(right_frame - left_frame);
      variant_type interpolated = left->value;
      assert(interpolated.index() == property().variant_value().index());
      for (std::size_t channel = 0; channel < n; ++channel) {
        const double left_value = get_channel_value(left->value, channel);
        const double right_value = get_channel_value(right->value, channel);
        const std::array<double, 4> segment {
          left_value,
          left_value + get_channel_value(left->right_offset, channel),
          right_value + get_channel_value(right->left_offset, channel),
          right_value
        };
        const double v = ::interpolate(segment, t, m_interpolation);
        set_channel_value(interpolated, channel, v);
      }
      assert(interpolated.index() == property().variant_value().index());
      return interpolated;
    }
  }
}

Track::Knot& Track::knot(int frame) const
{
  return *m_knots.at(frame);
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
  auto knot = std::move(m_knots.extract(old_frame).mapped());
  m_knots.insert({ new_frame, std::move(knot) });
}

void Track::insert_knot(int frame, std::unique_ptr<Knot> knot)
{
  assert(knot->value.index() == property().variant_value().index());
  assert(m_knots.find(frame) == m_knots.end());
  m_knots.insert({ frame, std::move(knot) });
}

QString Track::type() const
{
  static const QString property_suffix = "Property";
  const QString type = property().type();
  assert(type.size() > property_suffix.size());
  return type.mid(0, type.size() - property_suffix.size());
}

bool Track::is_consistent(int frame) const
{
  if (const auto it = m_knots.find(frame); it != m_knots.end()) {
    return it->second->value == property().variant_value();
  } else {
    return true;
  }
}

bool Track::is_numerical() const
{
  return property().is_numerical();
}

void Track::set_interpolation(Track::Interpolation interpolation)
{
  m_interpolation = interpolation;
}

Track::Interpolation Track::interpolation() const
{
  return m_interpolation;
}

Track::Knot::Knot(AbstractDeserializer& deserializer, const Pointer& pointer, const QString& type)
{
  if (type == "Reference") {
    m_reference_id = deserializer.get<std::size_t>(pointer);
    deserializer.register_reference_polisher(*this);
    value = nullptr;
  } else {
    value = deserializer.get(pointer, type);
    polish();
  }
}

Track::Knot::Knot(const variant_type& value) : value(value)
{
  polish();
}

void Track::Knot::swap(Track::Knot& other)
{
  std::swap(other.value, value);
  std::swap(other.left_offset, left_offset);
  std::swap(other.right_offset, right_offset);
  std::swap(other.m_reference_id, m_reference_id);
}

std::unique_ptr<Track::Knot> Track::Knot::clone() const
{
  return std::unique_ptr<Track::Knot>(new Track::Knot(*this));
}

void Track::Knot::update_references(const std::map<std::size_t, AbstractPropertyOwner*>& map)
{
  if (m_reference_id == 0) {
    value = nullptr;
  } else {
    value = map.at(m_reference_id);
  }
}

bool Track::Knot::operator==(const Track::Knot& other) const
{
  return value == other.value
      && left_offset == other.left_offset
      && right_offset == other.right_offset;
}

bool Track::Knot::operator!=(const Track::Knot& other) const
{
  return !(*this == other);
}

variant_type& Track::Knot::offset(Track::Knot::Side side)
{
  switch (side) {
  case Side::Left:
    return left_offset;
  case Side::Right:
    return right_offset;
  default:
    Q_UNREACHABLE();
    return right_offset;
  }
}

void Track::Knot::polish()
{
  std::visit([this](auto&& vv) {
    using T = std::decay_t<decltype(vv)>;
    if constexpr (n_channels<T>() > 0) {
      left_offset = null_value<T>;
      right_offset = null_value<T>;
    } else {
      // don't care about non numeric types.
    }
  }, value);
}

}  // namespace omm
