#pragma once

namespace omm
{

enum class Direction { Forward = 0, Backward = 1 };
constexpr Direction other(const Direction d)
{
  return static_cast<Direction>(1 - static_cast<int>(d));
}

}  // namespace omm
