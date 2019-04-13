#pragma once

#define GEOS_USE_ONLY_R_API
#include <geos_c.h>

namespace omm
{

class Geos
{
public:
  Geos();
  ~Geos();
  Geos(const Geos& other) = delete;
  Geos(Geos&& other) = delete;
  Geos& operator=(const Geos& other) = delete;
  Geos& operator=(Geos&& other) = delete;

  static Geos& instance();
  void finish() const;
  GEOSContextHandle_t handle() const;

private:
  static Geos* m_instance;
  GEOSContextHandle_t m_handle;
};

}  // namespace omm
