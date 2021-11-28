#include "common.h"

bool is_not_null(const void* p)
{
  return p != nullptr;
}

void hash_combine(std::size_t& hash, const std::size_t& new_hash)
{
  // https://stackoverflow.com/q/4948780
  hash ^= new_hash + 0x9e3779b9 + (hash << 6) + (hash >> 2);
}
