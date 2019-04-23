#pragma once

#include <sstream>
#include <cassert>
#include <memory>
#include <vector>

namespace omm
{

class AbstractSerializer;
class AbstractDeserializer;

class Serializable
{
public:
  using Pointer = std::string;
  using ByteArray = std::vector<char>;
  virtual ~Serializable() = default;

  virtual void serialize(AbstractSerializer&, const Pointer&) const {}
  virtual void deserialize(AbstractDeserializer&, const Pointer&) {}

  template<typename PointerT> static auto make_pointer(const PointerT& pointer)
  {
    constexpr char SEPARATOR = '/';
    std::ostringstream ostream;
    ostream << pointer;
    const std::string str = ostream.str();
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
    std::string lhs = make_pointer<PointerT>(pointer);
    std::string rhs = make_pointer<PointerTs...>(pointers...);
    return lhs + rhs;
  }
};

}  // omm
