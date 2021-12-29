#pragma once

#include "common.h"

namespace Geom
{
class PathVector;
class Path;
}  // namespace Geom

namespace omm
{

class PathVector;
class Path;

[[nodiscard]] Geom::PathVector omm_to_geom(const PathVector& path_vector,
                                           InterpolationMode interpolation = InterpolationMode::Bezier);
[[nodiscard]] Geom::Path omm_to_geom(const Path& path_vector,
                                     InterpolationMode interpolation = InterpolationMode::Bezier);
[[nodiscard]] std::unique_ptr<PathVector> geom_to_omm(const Geom::PathVector& path_vector);
[[nodiscard]] std::unique_ptr<Path> geom_to_omm(const Geom::Path& geom_path, PathVector* parent);

}  // namespace omm
