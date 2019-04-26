#include <gtest/gtest.h>
#include <glog/logging.h>

#include "aspects/treeelement.h"
#include "common.h"
#include <QDebug>

namespace
{

using item_map = std::map<std::string, omm::TreeTestItem*>;

std::unique_ptr<omm::TreeTestItem> make_tree(int depth, int breadth, item_map& items,
                                             const std::string& path = "root")
{
  assert(depth >= 0);

  auto root = std::make_unique<omm::TreeTestItem>();
  root->name = path;
  items.insert(std::pair(root->name, root.get()));

  if (depth > 0) {
    for (int i = 0; i < breadth; ++i) {
      root->adopt(make_tree(depth-1, breadth, items, path + "/" + std::to_string(i)));
    }
  }
  return root;
}

}

TEST(tree, lca)
{
  item_map items;
  auto root = make_tree(3, 3, items);

  static const auto lca = [&items](const std::string a, const std::string& b) {
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

  const auto test_sort = [&items](const std::set<std::string>& item_names,
                                  const std::vector<std::string>& sorted_item_names) {
    const auto transform = [&items](const auto& item_names) {
      return ::transform<omm::TreeTestItem*>(item_names, [&items](const std::string& name) {
        return items[name];
      });
    };

    EXPECT_EQ(omm::TreeTestItem::sort(transform(item_names)), transform(sorted_item_names));
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
