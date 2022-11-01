#include "path/ccwcomparator.h"
#include "geometry/point.h"
#include "path/edge.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"
#include "testutil.h"
#include "gtest/gtest.h"

namespace
{

class PathFactory
{
public:
  explicit PathFactory(omm::PathVector& path_vector) : m_path_vector(path_vector), m_hinge(make_path_point({0, 0}))
  {
  }

  omm::Edge* make_trunk()
  {
    auto& trunk = add_path();
    m_base = &trunk.add_edge(make_path_point({-1, 0}), m_hinge);
    return &trunk.add_edge(m_hinge, make_path_point({1, 0}));
  }

  omm::Edge* straight(const double angle) const
  {
    return &add_path().add_edge(m_hinge, make_path_point({std::cos(angle), std::sin(angle)}));
  }

  omm::Edge* straight_out(const double first_tangent_length, const omm::Vec2f& second_tangent_pos,
                          const omm::Vec2f& second_point_pos) const
  {
    auto& path = add_path();
    auto second_point = make_path_point(second_point_pos);
    second_point->geometry().set_tangent_position({&path, omm::Direction::Backward}, second_tangent_pos);
    m_hinge->geometry().set_tangent({&path, omm::Direction::Forward}, omm::PolarCoordinates(0, first_tangent_length));
    return &path.add_edge(m_hinge, second_point);
  }

  omm::Edge* base() const
  {
    return m_base;
  }

private:
  omm::Path& add_path() const
  {
    return m_path_vector.add_path();
  }

  std::shared_ptr<omm::PathPoint> make_path_point(const omm::Vec2f& pos) const
  {
    return std::make_shared<omm::PathPoint>(omm::Point(pos), &m_path_vector);
  }

  omm::Edge* m_base;
  omm::PathVector& m_path_vector;
  std::shared_ptr<omm::PathPoint> m_hinge;
};

}  // namespace

TEST(CCWComparatorTest, A)
{
  auto pv = omm::PathVector();
  std::deque<omm::Edge*> edges;

  PathFactory path_factory(pv);
  edges.emplace_back(path_factory.make_trunk());

  edges.emplace_front(path_factory.straight(-45 * M_PI / 180.0));

  edges.emplace_back(path_factory.straight(45 * M_PI / 180.0));
  edges.emplace_back(path_factory.straight_out(0.2, {0.3, 0.0}, {1.0, 1.0}));
  edges.emplace_back(path_factory.straight_out(0.2, {0.5, 0.0}, {1.0, 1.0}));
  edges.emplace_back(path_factory.straight_out(0.2, {0.3, 0.0}, {1.0, 2.0}));
  edges.emplace_back(path_factory.straight_out(0.2, {0.5, 0.0}, {1.0, 2.0}));

  ommtest::Application app;
  pv.to_svg("/tmp/" + ommtest::Application::test_id_for_filename() + ".svg");

  // Check if edges are ascending.
  // Only checking if edges is sorted is not sufficient, we want to compare each item with every item to
  // make sure that the order incuded by CCWComparator is well-defined.
  const omm::CCWComparator ccw_comparator(omm::DEdge::fwd(path_factory.base()));
  for (std::size_t i = 0; i < edges.size(); ++i) {
    for (std::size_t j = 0; j < edges.size(); ++j) {
      const auto a = omm::DEdge::fwd(edges.at(i));
      const auto b = omm::DEdge::fwd(edges.at(j));
      const auto comp_result = ccw_comparator(a, b);
      static constexpr auto to_string = [](const auto& edge) { return edge.to_string().toStdString(); };
      ASSERT_EQ(comp_result, i < j) << to_string(a) << " < " << to_string(b);
    }
  }
}
