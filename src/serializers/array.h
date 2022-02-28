#pragma once

#include <memory>

namespace omm::serialization
{

template<typename Worker>
class Array
{
public:
  Array(Worker& parent) : m_parent(parent) {}
  virtual ~Array() { m_parent.end_array(); }
  virtual Worker& next() = 0;

protected:
  std::size_t m_next_index = 0;
  Worker& m_parent;
  std::unique_ptr<Worker> m_current;
};

class DeserializerWorker;
class DeserializationArray : public Array<DeserializerWorker>
{
public:
  using Array::Array;
  virtual std::size_t size() const = 0;
};

class SerializerWorker;
class SerializationArray : public Array<SerializerWorker>
{
public:
  using Array::Array;
};

}  // namespace omm::serialization
