#pragma once

#include "geometry/point.h"
#include "geometry/vec2.h"
#include "path/dedge.h"
#include <QApplication>
#include <array>
#include <deque>
#include <memory>
#include <vector>

namespace omm
{

class Application;
class PythonEngine;
class Options;
class PathVector;
}  // namespace omm

namespace ommtest
{

template<std::size_t N>
std::vector<char*> string_array_to_charpp(std::array<std::string, N>& string_array)
{
  std::vector<char*> vs;
  vs.reserve(N);
  for (std::size_t i = 0; i < N; ++i) {
    vs.push_back(string_array.at(i).data());
  }
  return vs;
}

class Application
{
public:
  explicit Application();
  ~Application();
  omm::Application& omm_app() const;

private:
  std::array<std::string, 3> argv_{"test", "-platform", "offscreen"};
  std::vector<char*> argv = string_array_to_charpp(argv_);
  int argc = argv.size();
  QApplication m_q_application;
  std::unique_ptr<omm::Application> m_omm_application;
};

bool have_opengl();

#define SKIP_IF_NO_OPENGL \
  do { \
    if (!ommtest::have_opengl()) { \
      GTEST_SKIP(); \
    } \
  } while (false)

class PathVectorHeap
{
protected:
  omm::PathVector* annex(std::unique_ptr<omm::PathVector> pv);

private:
  // The TestCase objects are not persistent, hence the path vectors need to be stored beyond the
  // lifetime of the test case.
  static std::list<std::unique_ptr<omm::PathVector>> m_path_vectors;
};

struct EllipseMaker
{
public:
  omm::Vec2f origin;
  omm::Vec2f radii;
  std::size_t point_count;
  bool closed;
  bool no_tangents;

  std::deque<omm::DEdge> edges = {};
  [[nodiscard]] omm::Point ith_point(const std::size_t i) const;
  omm::Path& make_path(omm::PathVector& pv);
  [[nodiscard]] std::string to_string() const;
  [[nodiscard]] std::set<std::deque<omm::DEdge>> faces() const;
  [[nodiscard]] double area() const;
};

}  // namespace ommtest
