#include "dnf.h"
#include "common.h"
#include <gtest/gtest.h>

namespace
{
enum class Zoo { None = 0x0, Lion = 0x1, Zebra = 0x2, Horse = 0x4, Fox = 0x8,
                 Hedgehog = 0x10, Shark = 0x20, Ant = 0x40 };
}

template<> struct omm::EnableBitMaskOperators<Zoo> : std::true_type {};

TEST(dnf, one_minterm)
{
  using namespace omm;

  {
    DNF<Zoo> dnf { { } };
    EXPECT_TRUE(dnf.evaluate(Zoo::Lion | Zoo::Zebra));
    EXPECT_TRUE(dnf.evaluate(Zoo::Horse | Zoo::Fox));
    EXPECT_TRUE(dnf.evaluate(Zoo::Lion | Zoo::Zebra | Zoo::Shark));
    EXPECT_TRUE(dnf.evaluate(Zoo::Lion | Zoo::Zebra | Zoo::Ant | Zoo::Hedgehog));
    EXPECT_TRUE(dnf.evaluate(Zoo::Lion | Zoo::Fox));
    EXPECT_TRUE(dnf.evaluate(Zoo::Lion | Zoo::Fox | Zoo::Shark));
    EXPECT_TRUE(dnf.evaluate(Zoo::Hedgehog | Zoo::Fox | Zoo::Zebra));
    EXPECT_TRUE(dnf.evaluate(Zoo::None));
  }

  {
    DNF<Zoo> dnf { { Zoo::Lion } };
    EXPECT_TRUE(dnf.evaluate(Zoo::Lion | Zoo::Zebra));
    EXPECT_FALSE(dnf.evaluate(Zoo::Horse | Zoo::Fox));
    EXPECT_TRUE(dnf.evaluate(Zoo::Lion | Zoo::Zebra | Zoo::Shark));
    EXPECT_TRUE(dnf.evaluate(Zoo::Lion | Zoo::Zebra | Zoo::Ant | Zoo::Hedgehog));
    EXPECT_TRUE(dnf.evaluate(Zoo::Lion | Zoo::Fox));
    EXPECT_TRUE(dnf.evaluate(Zoo::Lion | Zoo::Fox | Zoo::Shark));
    EXPECT_FALSE(dnf.evaluate(Zoo::Hedgehog | Zoo::Fox | Zoo::Zebra));
    EXPECT_FALSE(dnf.evaluate(Zoo::None));
  }

  {
    DNF<Zoo> dnf { { Zoo::Lion, Zoo::Horse, Zoo::Shark } };
    EXPECT_FALSE(dnf.evaluate(Zoo::Lion | Zoo::Zebra));
    EXPECT_FALSE(dnf.evaluate(Zoo::Horse | Zoo::Fox));
    EXPECT_FALSE(dnf.evaluate(Zoo::Lion | Zoo::Zebra | Zoo::Shark));
    EXPECT_FALSE(dnf.evaluate(Zoo::Lion | Zoo::Zebra | Zoo::Ant | Zoo::Hedgehog));
    EXPECT_FALSE(dnf.evaluate(Zoo::Lion | Zoo::Fox));
    EXPECT_FALSE(dnf.evaluate(Zoo::Lion | Zoo::Fox | Zoo::Shark));
    EXPECT_TRUE(dnf.evaluate(Zoo::Lion | Zoo::Horse | Zoo::Shark));
    EXPECT_TRUE(dnf.evaluate(Zoo::Lion | Zoo::Horse | Zoo::Shark | Zoo::Fox));
    EXPECT_FALSE(dnf.evaluate(Zoo::Hedgehog | Zoo::Fox | Zoo::Zebra));
    EXPECT_FALSE(dnf.evaluate(Zoo::None));
  }
}

TEST(dnf, two_minterms)
{
  using namespace omm;

  {
    DNF<Zoo> dnf { { }, { } };
    EXPECT_TRUE(dnf.evaluate(Zoo::Lion | Zoo::Zebra));
    EXPECT_TRUE(dnf.evaluate(Zoo::Horse | Zoo::Fox));
    EXPECT_TRUE(dnf.evaluate(Zoo::Lion | Zoo::Zebra | Zoo::Shark));
    EXPECT_TRUE(dnf.evaluate(Zoo::Lion | Zoo::Zebra | Zoo::Ant | Zoo::Hedgehog));
    EXPECT_TRUE(dnf.evaluate(Zoo::Lion | Zoo::Fox));
    EXPECT_TRUE(dnf.evaluate(Zoo::Lion | Zoo::Fox | Zoo::Shark));
    EXPECT_TRUE(dnf.evaluate(Zoo::Hedgehog | Zoo::Fox | Zoo::Zebra));
    EXPECT_TRUE(dnf.evaluate(Zoo::None));
  }

  {
    DNF<Zoo> dnf { { Zoo::Lion, Zoo::Zebra }, { } };
    EXPECT_TRUE(dnf.evaluate(Zoo::Lion | Zoo::Zebra));
    EXPECT_TRUE(dnf.evaluate(Zoo::Horse | Zoo::Fox));
    EXPECT_TRUE(dnf.evaluate(Zoo::Lion | Zoo::Zebra | Zoo::Shark));
    EXPECT_TRUE(dnf.evaluate(Zoo::Lion | Zoo::Zebra | Zoo::Ant | Zoo::Hedgehog));
    EXPECT_TRUE(dnf.evaluate(Zoo::Lion | Zoo::Fox));
    EXPECT_TRUE(dnf.evaluate(Zoo::Lion | Zoo::Fox | Zoo::Shark));
    EXPECT_TRUE(dnf.evaluate(Zoo::Hedgehog | Zoo::Fox | Zoo::Zebra));
    EXPECT_TRUE(dnf.evaluate(Zoo::None));
  }

  {
    DNF<Zoo> dnf { { Zoo::Lion, Zoo::Zebra }, { Zoo::Horse, Zoo::Fox, Zoo::Zebra } };
    EXPECT_TRUE(dnf.evaluate(Zoo::Lion | Zoo::Zebra));
    EXPECT_FALSE(dnf.evaluate(Zoo::Horse | Zoo::Fox));
    EXPECT_TRUE(dnf.evaluate(Zoo::Horse | Zoo::Fox | Zoo::Zebra));
    EXPECT_TRUE(dnf.evaluate(Zoo::Lion | Zoo::Zebra | Zoo::Shark));
    EXPECT_TRUE(dnf.evaluate(Zoo::Lion | Zoo::Zebra | Zoo::Ant | Zoo::Hedgehog));
    EXPECT_FALSE(dnf.evaluate(Zoo::Lion | Zoo::Fox));
    EXPECT_FALSE(dnf.evaluate(Zoo::Lion | Zoo::Fox | Zoo::Shark));
    EXPECT_FALSE(dnf.evaluate(Zoo::Hedgehog | Zoo::Fox | Zoo::Zebra));
    EXPECT_FALSE(dnf.evaluate(Zoo::None));
  }
}
