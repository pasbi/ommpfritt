#pragma once

#include <set>

template<typename T> using transparent_set = std::set<T, std::less<>>;
