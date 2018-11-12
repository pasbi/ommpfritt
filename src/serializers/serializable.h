#pragma once

#include <sstream>
#include <glog/logging.h>
#include <assert.h>

namespace omm
{

class AbstractSerializer;
class AbstractDeserializer;

class Serializable
{
public:
  using Pointer = std::string;
  using ByteArray = std::vector<char>;
  using IdType = std::size_t;

  virtual void serialize(AbstractSerializer& serializer, const Pointer& root) const {};
  virtual void deserialize(AbstractDeserializer& deserializer, const Pointer& root) {};
  IdType identify() const;

  template<typename PointerT> static auto make_pointer(const PointerT& pointer)
  {
    constexpr char SEPARATOR = '/';
    std::ostringstream ostream;
    ostream << "" << pointer;
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

protected:
  template<typename T, typename Serializer, typename Deserializer>
  static std::unique_ptr<T> copy(std::unique_ptr<T> copy, const T& self)
  {
    static constexpr auto POINTER = "copy";
    std::stringstream stream;
    {
      Serializer serializer(stream);
      self.serialize(serializer, POINTER);
    }
    {
      Deserializer deserializer(stream);
      copy->deserialize(deserializer, POINTER);
    }
    return copy;
  }
};

}  // omm
