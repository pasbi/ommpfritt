#pragma once

#include <sstream>
#include <glog/logging.h>
#include <cassert>
#include <memory>

namespace omm
{

class AbstractSerializer;
class AbstractDeserializer;

class Serializable
{
public:
  using Pointer = std::string;
  using ByteArray = std::vector<char>;

  virtual void serialize(AbstractSerializer& serializer, const Pointer& root) const {};
  virtual void deserialize(AbstractDeserializer& deserializer, const Pointer& root) {};

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
