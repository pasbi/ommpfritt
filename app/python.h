#pragma once

#include <string>

class Scene;

class Python
{
private:
  class ConstructorTag {
  public:
    explicit ConstructorTag() = default;
  };
public:
  Python(ConstructorTag);
  bool run(Scene& scene, const std::string code);
  static Python& instance();
};