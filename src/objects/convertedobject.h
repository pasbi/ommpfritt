#pragma once

#include <memory>

namespace omm
{

class Object;
class DisjointPathPointSetForest;

struct ConvertedObject
{
  ConvertedObject(std::unique_ptr<Object>&& object, bool keep_children);
  ConvertedObject(std::unique_ptr<Object>&& object,
                  std::unique_ptr<DisjointPathPointSetForest>&& joined_points,
                  bool keep_children);
  ConvertedObject(std::unique_ptr<Object>&& object,
                  DisjointPathPointSetForest&& joined_points,
                  bool keep_children);
  ~ConvertedObject();
  ConvertedObject(const ConvertedObject&) = delete;
  ConvertedObject(ConvertedObject&&) = delete;
  ConvertedObject& operator=(const ConvertedObject&) = delete;
  ConvertedObject& operator=(ConvertedObject&&) = delete;

  std::unique_ptr<Object> object;
  std::unique_ptr<DisjointPathPointSetForest> joined_points;
  bool keep_children;
};

}  // namespace omm
