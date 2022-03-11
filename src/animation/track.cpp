#include "animation/track.h"
#include "common.h"
#include "logging.h"
#include "properties/property.h"
#include "scene/scene.h"
#include "animation/knot.h"

namespace
{

using Interpolation = omm::Track::Interpolation;

double interpolate(const std::array<double, 4>& segment, double t, Interpolation interpolation)
{
  static constexpr double bc41 = 1.0;
  static constexpr double bc42 = 3.0;
  static constexpr double bc43 = 3.0;
  static constexpr double bc44 = 1.0;
  std::array<double, 4> bernstein4{bc41 * (1 - t) * (1 - t) * (1 - t),
                                   // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
                                   bc42 * t * (1 - t) * (1 - t),
                                   // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
                                   bc43 * t * t * (1 - t),
                                   bc44 * t * t * t};
  switch (interpolation) {
  case Interpolation::Step:
    return segment[0];
  case Interpolation::Linear:
    return (1.0 - t) * segment[0] + t * segment[3];
  case Interpolation::Bezier:
    return bernstein4[0] * segment[0] + bernstein4[1] * segment[1] + bernstein4[2] * segment[2]
           + bernstein4[3] * segment[3];
  default:
    Q_UNREACHABLE();
    return 0.0;
  }
}

struct KnotAt
{
  const omm::Knot* knot = nullptr;
  int frame = 0;
};

std::pair<KnotAt, KnotAt> find_left_right_knot(const int frame, const auto& knots)
{
  KnotAt left;
  KnotAt right;
  for (const auto& knot : knots) {
    if (knot.first <= frame) {
      left.frame = knot.first;
      left.knot = knot.second.get();
    } else {
      assert(knot.first > frame);
      right.frame = knot.first;
      right.knot = knot.second.get();
      break;
    }
  }
  return {left, right};
}

omm::variant_type interpolate(const Interpolation& interpolation,
                              const double frame,
                              const KnotAt& left,
                              const KnotAt& right)
{
  if (left.knot == nullptr && right.knot != nullptr) {
    return right.knot->value;
  } else if (left.knot != nullptr && right.knot == nullptr) {
    return left.knot->value;
  } else if (left.knot == nullptr && right.knot == nullptr) {
    LFATAL("Unexpected condition.");
    return {};
  } else {
    const std::size_t n = n_channels(left.knot->value);
    assert(n == n_channels(right.knot->value));
    if (n == 0) {
      return left.knot->value;  // non-numerical types cannot be interpolated.
    } else {
      const double t = (frame - left.frame) / static_cast<double>(right.frame - left.frame);
      auto interpolated = left.knot->value;
      for (std::size_t channel = 0; channel < n; ++channel) {
        const double left_value = get_channel_value(left.knot->value, channel);
        const double right_value = get_channel_value(right.knot->value, channel);
        const std::array<double, 4> segment{
            left_value,
            left_value + get_channel_value(left.knot->right_offset, channel),
            right_value + get_channel_value(right.knot->left_offset, channel),
            right_value};
        const double v = ::interpolate(segment, t, interpolation);
        set_channel_value(interpolated, channel, v);
      }
      return interpolated;
    }
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

Track::Track(Property& property) : m_property(property)
{
}

Track::~Track() = default;

std::unique_ptr<Track> Track::clone() const
{
  auto track = std::make_unique<Track>(m_property);
  for (auto&& [frame, knot] : m_knots) {
    track->m_knots.insert({frame, knot->clone()});
  }
  track->m_interpolation = m_interpolation;
  return track;
}

void Track::serialize(serialization::SerializerWorker& worker) const
{
  worker.sub(TYPE_KEY)->set_value(type());
  worker.sub(INTERPOLATION_KEY)->set_value(m_interpolation);

  const auto key_frames = this->key_frames();
  worker.sub(KNOTS_KEY)->set_value(key_frames, [this](const auto& key_frame, auto& worker_i) {
    const Knot& knot = *m_knots.at(key_frame);
    worker_i.sub(FRAME_KEY)->set_value(key_frame);
    worker_i.sub(VALUE_KEY)->set_value(knot.value);
    if (is_numerical()) {
      worker_i.sub(LEFT_VALUE_KEY)->set_value(knot.left_offset);
      worker_i.sub(RIGHT_VALUE_KEY)->set_value(knot.right_offset);
    }
  });
}

void Track::deserialize(serialization::DeserializerWorker& worker)
{
  const QString type = worker.sub(TYPE_KEY)->get_string();
  m_interpolation = worker.sub(INTERPOLATION_KEY)->get<Interpolation>();

  worker.sub(KNOTS_KEY)->get_items([this, type](auto& worker_i) {
    const int frame = worker_i.sub(FRAME_KEY)->get_int();
    auto knot = std::make_unique<Knot>(*worker_i.sub(VALUE_KEY), type);
    if (is_numerical()) {
      knot->left_offset = worker_i.sub(LEFT_VALUE_KEY)->get(type);
      knot->right_offset = worker_i.sub(RIGHT_VALUE_KEY)->get(type);
    }
    m_knots.insert(std::pair(frame, std::move(knot)));
  });
}

std::unique_ptr<Knot> Track::remove_knot(int frame)
{
  assert(m_knots.find(frame) != m_knots.end());
  return std::move(m_knots.extract(frame).mapped());
}

double Track::interpolate(double frame, std::size_t channel) const
{
  // this can be optimized!
  return get_channel_value(interpolate(frame), channel);
}

variant_type Track::interpolate(const double frame) const
{
  assert(!m_knots.empty());

  const auto iframe = static_cast<int>(frame);
  if (const auto it = m_knots.find(iframe); it != m_knots.end()) {
    return it->second->value;
  }

  const auto& [left, right] = ::find_left_right_knot(iframe, m_knots);
  return ::interpolate(m_interpolation, frame, left, right);
}

Knot& Track::knot(int frame) const
{
  return *m_knots.at(frame);
}

std::vector<int> Track::key_frames() const
{
  return util::transform<std::vector>(m_knots, [](auto&& v) { return v.first; });
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
  m_knots.insert({new_frame, std::move(knot)});
}

void Track::insert_knot(int frame, std::unique_ptr<Knot> knot)
{
  assert(knot->value.index() == property().variant_value().index());
  assert(m_knots.find(frame) == m_knots.end());
  m_knots.insert({frame, std::move(knot)});
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

}  // namespace omm
