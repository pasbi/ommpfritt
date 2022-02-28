#pragma once

#include <QString>
#include <cassert>
#include <memory>
#include <sstream>
#include <vector>

namespace omm
{

namespace serialization
{
class SerializerWorker;
class DeserializerWorker;
}  // namespace serialization

class Serializable
{
public:
  virtual ~Serializable() = default;
  Serializable(Serializable&&) = default;
  Serializable(const Serializable&) = default;
  Serializable& operator=(Serializable&&) = default;
  Serializable& operator=(const Serializable&) = default;
  Serializable() = default;

  virtual void deserialize(serialization::DeserializerWorker&) = 0;
  virtual void serialize(serialization::SerializerWorker&) const = 0;
};

}  // namespace omm
