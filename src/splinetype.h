#pragma once

#include "common.h"
#include <QObject>
#include <map>

namespace omm
{
class SplineType
{
public:
  struct Knot {
    enum class Side { Left = 0x1, Middle = 0x2, Right = 0x4 };
    explicit Knot(double value = 0, double left_offset = 0, double right_offset = 0);
    double value;
    double left_offset;
    double right_offset;
    bool operator==(const Knot& other) const;
    bool operator!=(const Knot& other) const
    {
      return !(*this == other);
    }
    bool operator<(const Knot& other) const;
    [[nodiscard]] double get_value(Side side) const;
    void set_value(Side side, double value);
  };

  using knot_map_type = std::multimap<double, Knot>;
  knot_map_type knots;

private:
  template<typename Knots, typename Iterator> struct ControlPoint_ {
    using Side = Knot::Side;
    explicit ControlPoint_(Knots& knots, Iterator it, Side side)
        : m_knots(&knots), m_value({it, side})
    {
    }

    explicit ControlPoint_() = default;

    [[nodiscard]] bool is_valid() const
    {
      if (!m_value.has_value()) {
        return false;
      } else if (m_knots == nullptr) {
        return false;
      } else if (m_knots->end() == m_value.value().first) {
        return false;
      } else if (std::prev(m_knots->end()) == m_value.value().first && side() == Side::Right) {
        return false;
      } else if (m_knots->begin() == m_value.value().first && side() == Side::Left) {
        return false;
      } else {
        return true;
      }
    }

    [[nodiscard]] Knot& knot() const
    {
      return m_value.value().first->second;
    }
    [[nodiscard]] double& t() const
    {
      return m_value.value().first->first;
    }
    Knot::Side& side()
    {
      return m_value.value().second;
    }
    [[nodiscard]] Knot::Side side() const
    {
      return m_value.value().second;
    }
    Iterator& iterator()
    {
      return m_value.value().first;
    }
    [[nodiscard]] Iterator iterator() const
    {
      return m_value.value().first;
    }

    void advance()
    {
      if (m_value.has_value()) {
        auto& [it, side] = m_value.value();
        switch (side) {
        case Side::Left:
          side = Side::Middle;
          return;
        case Side::Middle:
          side = Side::Right;
          return;
        case Side::Right:
          std::advance(it, 1);
          side = Side::Left;
          return;
        default:
          Q_UNREACHABLE();
        }
      }
    }

  private:
    Knots* m_knots = nullptr;
    std::optional<std::pair<Iterator, Knot::Side>> m_value = std::nullopt;
  };

public:
  using ControlPoint = ControlPoint_<knot_map_type, knot_map_type::iterator>;
  ControlPoint begin();
  static ControlPoint invalid()
  {
    return ControlPoint();
  }

  enum class Initialization { Linear, Ease, Valley };

  explicit SplineType(const std::multimap<double, Knot>& knots);
  explicit SplineType(Initialization initialization, bool flip);
  explicit SplineType() = default;

  bool operator==(const SplineType& other) const;
  bool operator!=(const SplineType& other) const
  {
    return !(*this == other);
  }
  bool operator<(const SplineType& other) const;
  knot_map_type::iterator move(knot_map_type::const_iterator it, double new_t);
  static constexpr auto TYPE = "SplineType";

  struct Interpolation {
    std::optional<std::pair<double, Knot>> right;
    std::optional<std::pair<double, Knot>> left;
    double t;
    [[nodiscard]] double local_t() const;
    [[nodiscard]] double value() const;
    [[nodiscard]] double derivative() const;
    [[nodiscard]] std::array<double, 4> coefficients() const;
  };

  [[nodiscard]] Interpolation evaluate(double t) const;

Q_SIGNALS:
  void value_changed();
};

}  // namespace omm

template<> struct omm::EnableBitMaskOperators<omm::SplineType::Knot::Side> : std::true_type {
};
