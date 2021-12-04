#pragma once

#include <memory>

namespace omm
{

class Object;
class DisjointPathPointSetForest;

class ConvertedObject
{
public:
  ConvertedObject(std::unique_ptr<Object>&& object, bool keep_children);
  ~ConvertedObject();
  ConvertedObject(const ConvertedObject&) = delete;
  ConvertedObject(ConvertedObject&&) = delete;
  ConvertedObject& operator=(const ConvertedObject&) = delete;
  ConvertedObject& operator=(ConvertedObject&&) = delete;

  [[nodiscard]] std::unique_ptr<Object> extract_object();
  [[nodiscard]] Object& object() const;
  [[nodiscard]] bool keep_children() const;

private:
  std::unique_ptr<Object> m_object;
  bool m_keep_children;
};

}  // namespace omm
