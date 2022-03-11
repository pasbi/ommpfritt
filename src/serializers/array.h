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
  Array(const Array&) = delete;
  Array(Array&&) = delete;
  Array& operator=(const Array&) = delete;
  Array& operator=(Array&&) = delete;
  [[nodiscard]] virtual Worker& next()
  {
    m_current = m_parent.sub(m_next_index);
    m_next_index += 1;
    return *m_current;
  }

protected:
  std::size_t m_next_index = 0;
  Worker& m_parent;
  std::unique_ptr<Worker> m_current;
};

class DeserializerWorker;
class DeserializationArray : public Array<DeserializerWorker>
{
public:
  explicit DeserializationArray(DeserializerWorker& parent, std::size_t size);
  [[nodiscard]] std::size_t size() const;

private:
  const std::size_t m_size;
};

class SerializerWorker;
using SerializationArray = Array<SerializerWorker>;

}  // namespace omm::serialization
