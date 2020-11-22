#include "gtest/gtest.h"

#include "aspects/treeelement.h"
#include "common.h"
#include <QDebug>

namespace
{

using item_map = std::map<QString, omm::TreeTestItem*>;

std::unique_ptr<omm::TreeTestItem> make_tree(int depth, int breadth, item_map& items,
                                             const QString& path = "root")
{
  assert(depth >= 0);

  auto root = std::make_unique<omm::TreeTestItem>();
  root->name = path;
  items.insert(std::pair(root->name, root.get()));

  if (depth > 0) {
    for (int i = 0; i < breadth; ++i) {
      root->adopt(make_tree(depth-1, breadth, items, path + QString("/%1").arg(i)));
    }
  }
  return root;
}

template<typename ItemTs> auto get_items(const item_map& item_names, ItemTs&& items)
{
  return ::transform<omm::TreeTestItem*>(items, [&item_names](const QString& name) {
    return item_names.at(name);
  });
}

}

TEST(tree, lca)
{
  item_map items;
  auto root = make_tree(3, 3, items);

  const auto lca = [&items](const QString a, const QString& b) {
    return omm::TreeTestItem::lowest_common_ancestor(items[a], items[b]);
  };

  EXPECT_EQ(lca("root", "root/1"), items["root"]);
  EXPECT_EQ(lca("root", "root/0"), items["root"]);
  EXPECT_EQ(lca("root/0", "root"), items["root"]);
  EXPECT_EQ(lca("root/2", "root"), items["root"]);
  EXPECT_EQ(lca("root/0", "root/1"), items["root"]);
  EXPECT_EQ(lca("root/1", "root/2"), items["root"]);
  EXPECT_EQ(lca("root/1", "root/1"), items["root/1"]);
  EXPECT_EQ(lca("root/1/2", "root/1"), items["root/1"]);
  EXPECT_EQ(lca("root/1/2", "root/2/1"), items["root"]);
  EXPECT_EQ(lca("root/1/2", "root/2/2"), items["root"]);
  EXPECT_EQ(lca("root/0/2", "root/0/0"), items["root/0"]);
  EXPECT_EQ(lca("root/0/1", "root/0/1"), items["root/0/1"]);
  EXPECT_EQ(lca("root/1/0", "root/1/0"), items["root/1/0"]);
  EXPECT_EQ(lca("root/2/2", "root/2/2"), items["root/2/2"]);
}

TEST(tree, sort)
{
  item_map items;
  auto root = make_tree(3, 3, items);

  const auto test_sort = [&items](const std::set<QString>& item_names,
                                  const std::vector<QString>& sorted_item_names) {

    EXPECT_EQ(omm::TreeTestItem::sort(get_items(items, item_names)),
              get_items(items, sorted_item_names) );
  };

  test_sort({ "root" }, { "root" });
  test_sort({ "root/0", "root" }, { "root/0", "root" });
  test_sort({ "root", "root/0" }, { "root/0", "root" });
  test_sort({ "root", "root/0", "root/1" }, { "root/1", "root/0", "root" });
  test_sort({ "root/1", "root", "root/0" }, { "root/1", "root/0", "root" });
  test_sort({ "root", "root/2/1", "root/0" }, { "root/2/1", "root/0", "root" });
  test_sort({ "root", "root/0", "root/0/1", "root/1/1", "root/1/2" },
            { "root/1/2", "root/1/1", "root/0/1", "root/0", "root" });
  test_sort({ "root/2/2", "root/0", "root/0/1", "root/1/1", "root/1/2" },
            { "root/2/2", "root/1/2", "root/1/1", "root/0/1", "root/0" });
}

TEST(tree, remove_children)
{
  item_map items;
  auto root = make_tree(3, 3, items);

  auto test_remove_children = [&items](const std::set<QString>& candidate_names,
                                       const std::set<QString>& gt_names) {
    auto candidates = get_items(items, candidate_names);
    omm::TreeTestItem::remove_internal_children(candidates);
    const auto gt_items = get_items(items, gt_names);
    std::cout << "Expected: " << gt_items << std::endl;
    std::cout << "Actual:   " << candidates << std::endl;
    EXPECT_EQ(candidates, gt_items);
  };

  test_remove_children( {}, {} );
  test_remove_children( { "root" }, { "root" } );
  test_remove_children( { "root", "root/0" }, { "root" } );
  test_remove_children( { "root", "root/0", "root/1", "root/2" }, { "root" } );
  test_remove_children( { "root", "root/1", "root/2" }, { "root" } );
  test_remove_children( { "root/1", "root/2" }, { "root/1", "root/2" } );
  test_remove_children( { "root/0", "root/2" }, { "root/0", "root/2" } );
  test_remove_children( { "root/1/0", "root/0/1" }, { "root/1/0", "root/0/1" } );
  test_remove_children( { "root/1/0", "root/1/1" }, { "root/1/0", "root/1/1" } );
  test_remove_children( { "root/1/0", "root/0/1", "root/1" }, { "root/1", "root/0/1" } );
}
