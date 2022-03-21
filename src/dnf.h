#pragma once

#include "logging.h"
#include "serializers/serializerworker.h"
#include "serializers/deserializerworker.h"
#include <bitset>
#include <set>

namespace omm
{
namespace DNF_detail
{
template<typename E>
using underlying_type_t = std::conditional_t<std::is_enum_v<E>, std::underlying_type_t<E>, E>;

template<typename T> struct DisjunctionImpl {
  template<typename V> static bool evaluate(const std::set<T>& s, const V& v)
  {
    return std::any_of(s.begin(), s.end(), [v](const T& term) { return term.evaluate(v); });
  }
  static constexpr auto operator_symbol = "∨";
};

template<typename T> struct ConjunctionImpl {
  template<typename V> static bool evaluate(const std::set<T>& s, const V& v)
  {
    return std::all_of(s.begin(), s.end(), [v](const T& term) { return term.evaluate(v); });
  }
  static constexpr auto operator_symbol = "∧";
};

}  // namespace DNF_detail

template<typename E> class Literal
{
  static std::size_t from_enum(E e)
  {
    using U = DNF_detail::underlying_type_t<E>;
    static constexpr auto n = std::numeric_limits<U>::digits;
    static_assert(n > 0);
    const auto v = static_cast<U>(e);
    assert(std::bitset<n>(v).count() == 1);
    for (std::size_t i = 0; i < n; ++i) {
      if (v & (1 << i)) {
        return i;
      }
    }
    Q_UNREACHABLE();
    return -1;
  }

public:
  explicit Literal(std::size_t i, bool value = true) : i(i), value(value)
  {
  }
  Literal(E e, bool value = true) : i(from_enum(e)), value(value)
  {
  }
  Literal() = default;

  [[nodiscard]] bool evaluate(const std::vector<bool>& value) const
  {
    return value[i] == this->value;
  }

  [[nodiscard]] bool evaluate(const E& e) const
  {
    const auto v = static_cast<DNF_detail::underlying_type_t<E>>(e);
    return !!(v & (1 << i)) == value;
  }

  void serialize(serialization::SerializerWorker& worker) const
  {
    worker.sub("i")->set_value(i);
    worker.sub("v")->set_value(value);
  }

  void deserialize(serialization::DeserializerWorker& worker)
  {
    i = worker.sub("i")->get_size_t();
    value =worker.sub("v")->get_bool();
  }

  [[nodiscard]] bool operator==(const Literal<E>& other) const
  {
    return i == other.i && value == other.value;
  }

  [[nodiscard]] bool operator<(const Literal<E>& other) const
  {
    if (i == other.i) {
      return value < other.value;
    } else {
      return i < other.i;
    }
  }

  [[nodiscard]] bool is_valid() const
  {
    return i != std::size_t(-1);
  }
  operator E() const
  {
    return static_cast<E>(1 << i);
  }

  [[nodiscard]] QString to_string() const
  {
    QString s;
    if (!value) {
      s += "¬";
    }
    if constexpr (std::is_enum_v<E>) {
      s += QString("%1").arg(static_cast<std::underlying_type_t<E>>(i));
    } else {
      s += QString("%1").arg(value ? 1 : 0);
    }

    return s;
  }

  std::size_t i = -1;
  bool value = false;
};

namespace DNF_detail
{
template<typename E, typename T, typename Junction> class Term
{
  static constexpr bool is_top_level = std::is_same_v<T, Literal<E>>;
  using U = underlying_type_t<E>;
  static std::set<Literal<E>> convert_literals(E positives, E negatives)
  {
    static constexpr std::size_t n = std::numeric_limits<U>::digits;
    std::set<Literal<E>> literals;

    const auto convert = [&literals](const U& arg, bool value) {
      for (std::size_t i = 0; i < n; ++i) {
        if (arg & (1 << i)) {
          literals.insert(Literal<E>(i, value));
        }
      }
    };

    convert(static_cast<U>(positives), true);
    convert(static_cast<U>(negatives), false);

    return literals;
  }

public:
  Term(std::initializer_list<T> ts) : terms(ts)
  {
  }

  Term() = default;

  Term(E positives, E negatives) requires is_top_level
      : terms(convert_literals(positives, negatives))
  {
  }

  template<typename V> [[nodiscard]] bool evaluate(const V& value) const
  {
    return Junction::evaluate(terms, value);
  }

  void serialize(serialization::SerializerWorker& worker) const
  {
    worker.sub("terms")->set_value(terms);
  }

  void deserialize(serialization::DeserializerWorker& worker)
  {
    worker.sub("terms")->get(terms);
  }

  [[nodiscard]] bool operator==(const Term<E, T, Junction>& other) const
  {
    return terms == other.terms;
  }

  [[nodiscard]] bool operator<(const Term<E, T, Junction>& other) const
  {
    return std::lexicographical_compare(terms.begin(),
                                        terms.end(),
                                        other.terms.begin(),
                                        other.terms.end());
  }

  [[nodiscard]] QString to_string() const
  {
    QString s;
    s += "( ";
    auto it = terms.begin();
    while (it != terms.end()) {
      if (it != terms.begin()) {
        s += QString{" %1 "}.arg(Junction::operator_symbol);
      }
      s += it->to_string();
      it++;
    }
    s += ") ";
    return s;
  }

  std::set<T> terms;
};

}  // namespace DNF_detail

template<typename E, typename T = Literal<E>>
using Conjunction = DNF_detail::Term<E, T, DNF_detail::ConjunctionImpl<T>>;
template<typename E, typename T = Literal<E>>
using Disjunction = DNF_detail::Term<E, T, DNF_detail::DisjunctionImpl<T>>;
template<typename E> using DNF = Disjunction<E, Conjunction<E>>;

}  // namespace omm
