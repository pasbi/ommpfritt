#pragma once

#include "variant.h"
#include <QString>
#include <cassert>
#include <memory>
#include <sstream>
#include <vector>

namespace omm
{
class AbstractSerializer;
class AbstractDeserializer;

class Serializable
{
public:
  using Pointer = QString;
  using ByteArray = std::vector<char>;
  virtual ~Serializable() = default;
  Serializable(Serializable&&) = default;
  Serializable(const Serializable&) = default;
  Serializable& operator=(Serializable&&) = default;
  Serializable& operator=(const Serializable&) = default;
  Serializable() = default;

  virtual void serialize(AbstractSerializer&, const Pointer&) const
  {
  }

  virtual void deserialize(AbstractDeserializer&, const Pointer&)
  {
  }

  template<typename PointerT> static QString make_pointer(const PointerT& pointer)
  {
    constexpr char SEPARATOR = '/';
    std::ostringstream ostream;
    ostream << pointer;
    auto str = QString::fromStdString(ostream.str());
    assert(str.size() > 0);
    if (str.at(0) == SEPARATOR) {
      return str;
    } else {
      return SEPARATOR + str;
    }
  }

  template<typename PointerT, typename... PointerTs>
  static auto make_pointer(const PointerT& pointer, const PointerTs&... pointers)
  {
    QString lhs = make_pointer<PointerT>(pointer);
    QString rhs = make_pointer<PointerTs...>(pointers...);
    return lhs + rhs;
  }
};

}  // namespace omm
