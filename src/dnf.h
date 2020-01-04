#pragma once

#include "logging.h"
#include <bitset>
#include "serializers/abstractserializer.h"
#include <set>
#include "aspects/serializable.h"

namespace omm
{

namespace DNF_detail
{

template<typename E> using underlying_type_t = std::conditional_t<std::is_enum_v<E>,
                                                                  std::underlying_type_t<E>,
                                                                  E>;

template<typename T> struct DisjunctionImpl
{
  template<typename V> static bool evaluate(const std::set<T>& s, const V& v)
  {
    return std::any_of(s.begin(), s.end(), [v](const T& term) { return term.evaluate(v); });
  }
  static constexpr auto operator_symbol = "∨";
};

template<typename T> struct ConjunctionImpl
{
  template<typename V> static bool evaluate(const std::set<T>& s, const V& v)
  {
    return std::all_of(s.begin(), s.end(), [v](const T& term) { return term.evaluate(v); });
  }
  static constexpr auto operator_symbol = "∧";
};

}  // namespace DNF_detail

template<typename E> class Literal : public Serializable
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
  explicit Literal(std::size_t i, bool value = true) : i(i), value(value) {}
  Literal(E e, bool value = true) : i(from_enum(e)), value(value) {}
  Literal() {}

  bool evaluate(const std::vector<bool>& value) const
  {
    return value[i] == this->value;
  }

  bool evaluate(const E& e) const
  {
    const auto v = static_cast<DNF_detail::underlying_type_t<E>>(e);
    return !!(v & (1 << i)) == value;
  }

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override
  {
    serializer.set_value(i, make_pointer(root, "i"));
    serializer.set_value(value, make_pointer(root, "v"));
  }

  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override
  {
    i = deserializer.get_size_t(make_pointer(root, "i"));
    value = deserializer.get_bool(make_pointer(root, "v"));
  }

  bool operator==(const Literal<E>& other) const
  {
    return i == other.i && value == other.value;
  }

  bool operator<(const Literal<E>& other) const
  {
    if (i == other.i) {
      return value < other.value;
    } else {
      return i < other.i;
    }
  }

  bool is_valid() const { return i != std::size_t(-1); }
  operator E() const { return static_cast<E>(1 << i); }

  std::size_t i = -1;
  bool value = false;
};

namespace DNF_detail {

template<typename E, typename T, typename Junction> class Term : public Serializable
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
  Term(std::initializer_list<T> ts) : terms(ts) {}
  Term() {}
  template<typename = typename std::enable_if<is_top_level>> Term(E positives, E negatives)
    : terms(convert_literals(positives, negatives))
  {
  }

  template<typename V> bool evaluate(const V& value) const
  {
    return Junction::evaluate(terms, value);
  }

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override
  {
    serializer.set_value(terms, make_pointer(root, "terms"));
  }

  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override
  {
    deserializer.get(terms, make_pointer(root, "terms"));
  }

  bool operator==(const Term<E, T, Junction>& other) const
  {
    return terms == other.terms;
  }

  bool operator<(const Term<E, T, Junction>& other) const
  {
    return std::lexicographical_compare(terms.begin(), terms.end(),
                                        other.terms.begin(), other.terms.end());
  }

  std::set<T> terms;
};

}  // namespace DNF_detail

template<typename E, typename T = Literal<E>> using Conjunction = DNF_detail::Term<E, T, DNF_detail::ConjunctionImpl<T>>;
template<typename E, typename T = Literal<E>> using Disjunction = DNF_detail::Term<E, T, DNF_detail::DisjunctionImpl<T>>;
template<typename E> using DNF = Disjunction<E, Conjunction<E>>;

template<typename E, typename T, typename Junction>
std::ostream& operator<<(std::ostream& ostream, const DNF_detail::Term<E, T, Junction>& dnf)
{
  ostream << "( ";
  auto it = dnf.terms.begin();
  while (it != dnf.terms.end()) {
    if (it != dnf.terms.begin()) {
      ostream << " " << Junction::operator_symbol << " ";
    }
    ostream << *it;
    it++;
  }
  ostream << ") ";
  return ostream;
}

template<typename E> std::ostream& operator<<(std::ostream& ostream, const Literal<E>& literal)
{
  if (!literal.value) {
    ostream << "¬";
  }
  if constexpr (std::is_enum_v<E>) {
    ostream << static_cast<std::underlying_type_t<E>>(literal.i);
  } else {
    ostream << static_cast<E>(literal);
  }

  return ostream;
}

}  // namespace omm
