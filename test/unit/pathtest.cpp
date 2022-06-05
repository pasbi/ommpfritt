#include "commands/addremovepointscommand.h"
#include "geometry/point.h"
#include "gtest/gtest.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"
#include "path/pathview.h"

namespace
{

bool check_correspondence(const auto& a, const std::vector<std::size_t>& a_indices,
                          const auto& b, const std::vector<std::size_t>& b_indices)
{
  assert(a_indices.size() == b_indices.size());
  for (std::size_t i = 0; i < a_indices.size(); ++i) {
    if (a.at(a_indices.at(i)) != b.at(b_indices.at(i))) {
      return false;
    }
  }
  return true;
}

bool equal(const auto& a, const auto& b)
{
  return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}


class PathAddPointsCommandTest : public ::testing::Test
{
protected:
  explicit PathAddPointsCommandTest()
      : m_path(m_path_vector.add_path())
  {
  }

  void SetUp()
  {
    ASSERT_EQ(m_path_vector.paths().size(), 1);
    EXPECT_EQ(m_path_vector.paths().front(), &m_path);
    EXPECT_EQ(m_path.points().size(), 0);
  }

  auto create_add_n_points_command(const std::size_t offset = 0, const std::size_t n = 1)
  {
    std::deque<std::shared_ptr<omm::PathPoint>> points;
    for (std::size_t i = 0; i < n; ++i) {
      points.emplace_back(std::make_unique<omm::PathPoint>(omm::Point{}, m_path.path_vector()));
    }
    omm::OwnedLocatedPath olp{&m_path, offset, points};
    std::deque<omm::OwnedLocatedPath> olps;
    olps.emplace_back(std::move(olp));
    return std::make_unique<omm::AddPointsCommand>(std::move(olps));
  }

  auto create_remove_single_point_front_command()
  {
    std::deque<omm::PathView> ptrs;
    ptrs.emplace_back(m_path, 0, 1);
    return std::make_unique<omm::RemovePointsCommand>(ptrs);
  }

  auto create_add_edge_command()
  {
    std::deque<std::shared_ptr<omm::PathPoint>> points;
    points.emplace_back(std::make_unique<omm::PathPoint>(omm::Point{}, m_path.path_vector()));
    points.emplace_back(std::make_unique<omm::PathPoint>(omm::Point{}, m_path.path_vector()));
    omm::OwnedLocatedPath olp{&m_path, 0, points};
    std::deque<omm::OwnedLocatedPath> olps;
    olps.emplace_back(std::move(olp));
    return std::make_unique<omm::AddPointsCommand>(std::move(olps));
  }

  omm::Path& path() const
  {
    return m_path;
  }

  bool has_distinct_points() const
  {
    const auto points = m_path.points();
    return std::set(points.begin(), points.end()).size() == points.size();
  }

private:
  omm::PathVector m_path_vector;
  omm::Path& m_path;
};

}  // namespace

TEST_F(PathAddPointsCommandTest, AddSinglePoint)
{
  auto add_single_point_command = create_add_n_points_command();

  add_single_point_command->redo();
  const auto one_point = path().points();
  ASSERT_EQ(one_point.size(), 1);
  add_single_point_command->undo();
  ASSERT_EQ(path().points().size(), 0);
  add_single_point_command->redo();
  ASSERT_EQ(path().points(), one_point);
}

TEST_F(PathAddPointsCommandTest, RemoveSinglePoint)
{
  auto add_single_point_command =  create_add_n_points_command();

  add_single_point_command->redo();
  const auto one_point = path().points();
  EXPECT_EQ(one_point.size(), 1);

  auto remove_single_point_command = create_remove_single_point_front_command();

  remove_single_point_command->redo();
  ASSERT_EQ(path().points().size(), 0);
  remove_single_point_command->undo();
  ASSERT_EQ(path().points(), one_point);
}

TEST_F(PathAddPointsCommandTest, AddTwoPoints)
{
  auto add_edge_command = create_add_edge_command();
  add_edge_command->redo();
  ASSERT_TRUE(has_distinct_points());
  const auto two_points = path().points();
  ASSERT_EQ(two_points.size(), 2);

  add_edge_command->undo();
  ASSERT_EQ(path().points().size(), 0);

  add_edge_command->redo();
  ASSERT_EQ(path().points(), two_points);
}

TEST_F(PathAddPointsCommandTest, Foo)
{
  auto a = create_add_n_points_command(0, 4);
  a->redo();
  ASSERT_EQ(path().points().size(), 4);
  const auto four_points = path().points();

  auto b = create_add_n_points_command(2, 3);
  b->redo();
  ASSERT_EQ(path().points().size(), 7);
  const auto seven_points = path().points();
  ASSERT_TRUE(check_correspondence(four_points, {0, 1, 2, 3}, seven_points, {0, 1, 5, 6}));
  ASSERT_EQ(b->owned_edges().size(), 1);

  b->undo();
  ASSERT_EQ(path().points(), four_points);

  a->undo();
  ASSERT_EQ(path().points().size(), 0);
}

TEST_F(PathAddPointsCommandTest, AddThreePointsFrontOneByOne)
{
  auto add_first_point_command = create_add_n_points_command();
  add_first_point_command->redo();
  const auto one_point = path().points();
  ASSERT_EQ(one_point.size(), 1);

  auto add_second_point_command = create_add_n_points_command();
  add_second_point_command->redo();
  const auto two_points = path().points();
  ASSERT_EQ(two_points.size(), 2);
  ASSERT_TRUE(has_distinct_points());

  auto add_third_point_command = create_add_n_points_command();
  add_third_point_command->redo();
  const auto three_points = path().points();
  ASSERT_EQ(three_points.size(), 3);
  ASSERT_TRUE(has_distinct_points());

  add_third_point_command->undo();
  ASSERT_EQ(path().points(), two_points);

  add_second_point_command->undo();
  ASSERT_EQ(path().points(), one_point);

  add_first_point_command->undo();
  ASSERT_EQ(path().points().size(), 0);

  add_first_point_command->redo();
  ASSERT_EQ(path().points(), one_point);

  add_second_point_command->redo();
  ASSERT_EQ(path().points(), two_points);

  add_third_point_command->redo();
  ASSERT_EQ(path().points(), three_points);
}

TEST_F(PathAddPointsCommandTest, AddPointsMiddle)
{
  auto add_first_4_points_command = create_add_n_points_command(0, 4);
  add_first_4_points_command->redo();
  ASSERT_TRUE(has_distinct_points());
  const auto four_points = path().points();
  ASSERT_EQ(four_points.size(), 4);

  auto add_1_point_front_command = create_add_n_points_command(0, 1);
  add_1_point_front_command->redo();
  ASSERT_TRUE(has_distinct_points());
  const auto five_points = path().points();
  ASSERT_EQ(five_points.size(), 5);
  ASSERT_TRUE(check_correspondence(five_points, {1, 2, 3, 4},
                                   four_points, {0, 1, 2, 3}));

  auto add_1_point_middle_command = create_add_n_points_command(1, 1);
  add_1_point_middle_command->redo();
  ASSERT_TRUE(has_distinct_points());
  const auto six_points = path().points();
  ASSERT_EQ(six_points.size(), 6);
  ASSERT_TRUE(check_correspondence(six_points, {0, 2, 3, 4, 5},
                                   five_points, {0, 1, 2, 3, 4}));

  auto add_3_point_middle_command = create_add_n_points_command(2, 3);
  add_3_point_middle_command->redo();
  ASSERT_TRUE(has_distinct_points());
  const auto nine_points = path().points();
  EXPECT_EQ(nine_points.size(), 9);
  ASSERT_TRUE(check_correspondence(nine_points, {0, 1, 5, 6, 7, 8},
                                   six_points,  {0, 1, 2, 3, 4, 5}));

  add_3_point_middle_command->undo();
  ASSERT_EQ(path().points(), six_points);

  add_1_point_middle_command->undo();
  ASSERT_EQ(path().points(), five_points);

  add_1_point_front_command->undo();
  ASSERT_EQ(path().points(), four_points);

  add_first_4_points_command->undo();
  ASSERT_EQ(path().points().size(), 0);
}
