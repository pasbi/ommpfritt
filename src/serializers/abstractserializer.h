#pragma once

#include <QObject>
#include <iosfwd>
#include <unordered_map>

#include "abstractfactory.h"
#include "common.h"
#include "geometry/vec2.h"
#include "variant.h"

namespace omm
{

class AbstractPropertyOwner;
class Color;
class ObjectTransformation;
class ReferenceProperty;
class Scene;
struct PolarCoordinates;

}  // namespace omm

namespace omm::serialization
{

class SerializerWorker;
class AbstractSerializer : public QObject
{
  Q_OBJECT
private:
  template<typename T, typename = void> struct is_iterable : std::false_type {
  };
  template<typename T>
  struct is_iterable<T,
                     std::void_t<decltype(std::declval<T>().size()),
                                 decltype(std::declval<T>().begin()),
                                 decltype(std::declval<T>().end())>> : std::true_type {
  };
  template<typename T> static inline constexpr bool is_iterable_v = is_iterable<T>::value;

public:
  class SerializeError : public std::runtime_error
  {
  public:
    using runtime_error::runtime_error;
  };

  virtual std::unique_ptr<SerializerWorker> worker() = 0;

protected:
  void register_serialzied_reference(AbstractPropertyOwner* reference);
};

}  // namespace omm::serialization
