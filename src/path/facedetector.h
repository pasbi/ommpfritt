#pragma once

#include <set>

namespace omm
{

class PathVector;
class Face;

std::set<Face> detect_faces(const PathVector& path_vector);

}  // namespace omm
