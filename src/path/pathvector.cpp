#include "path/pathvector.h"

#include "common.h"
#include "geometry/point.h"
#include "objects/pathobject.h"
#include "path/edge.h"
#include "path/face.h"
#include "path/facedetector.h"
#include "path/graph.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include "path/pathvectorisomorphism.h"
#include "path/pathvectorview.h"
#include "removeif.h"
#include <QObject>
#include <QPainter>
#include <QSvgGenerator>

namespace omm
{

class Style;

PathVector::PathVector(PathObject* path_object)
  : m_path_object(path_object)
{
}

PathVector::PathVector(const PathVector& other, PathObject* path_object)
  : m_path_object(path_object)
{
  copy_from(other);
}

PathVector::~PathVector() = default;

void PathVector::serialize(serialization::SerializerWorker& worker) const
{
  using PointIndices = std::map<const PathPoint*, std::size_t>;
  PointIndices point_indices;
  std::vector<std::vector<std::size_t>> iss;
  iss.reserve(m_paths.size());
  std::map<const Path*, std::size_t> path_indices;
  for (const auto& path : m_paths) {
    std::list<std::size_t> is;
    for (const auto* const point : path->points()) {
      const auto [it, was_inserted] = point_indices.try_emplace(point, point_indices.size());
      is.emplace_back(it->second);
    }
    iss.emplace_back(is.begin(), is.end());
    path_indices.emplace(path.get(), path_indices.size());
  }

  std::vector<const PathPoint*> point_indices_vec(point_indices.size());
  for (const auto& [point, index] : point_indices) {
    point_indices_vec.at(index) = point;
  }

  worker.sub("geometries")->set_value(point_indices_vec, [&path_indices](const PathPoint* const point, auto& worker) {
    point->geometry().serialize(worker, path_indices);
  });

  worker.sub("paths")->set_value(iss);
}

void PathVector::deserialize(serialization::DeserializerWorker& worker)
{
  std::vector<std::vector<std::size_t>> iss;
  worker.sub("paths")->get(iss);

  std::map<Path*, std::vector<std::size_t>> point_indices_per_path;
  std::vector<const Path*> paths;
  paths.reserve(iss.size());
  for (const auto& path_point_indices : iss) {
    auto& path = add_path();
    point_indices_per_path.emplace(&path, path_point_indices);
    paths.emplace_back(&path);
  }

  std::deque<std::shared_ptr<PathPoint>> points;
  worker.sub("geometries")->get_items([this, &paths, &points](auto& worker) {
    Point geometry;
    geometry.deserialize(worker, paths);
    points.emplace_back(std::make_shared<PathPoint>(geometry, this));
  });

  for (const auto& [path, point_indices] : point_indices_per_path) {
    if (point_indices.size() == 1) {
      path->set_single_point(points.at(point_indices.front()));
    } else {
      for (std::size_t i = 1; i < point_indices.size(); ++i) {
        const auto& a = points.at(point_indices.at(i - 1));
        const auto& b = points.at(point_indices.at(i));
        path->add_edge(std::make_unique<Edge>(a, b, path));
      }
    }
  }
}

QPainterPath PathVector::to_painter_path() const
{
  QPainterPath outline;
  for (const auto* path : paths()) {
    outline.addPath(path->to_painter_path());
  }
  return outline;
}

std::set<Face> PathVector::faces() const
{
  return FaceDetector(*this).faces();
}

std::vector<Edge*> PathVector::edges() const
{
  std::list<Edge*> edges;
  for (const auto& path : m_paths) {
    const auto pedges = path->edges();
    edges.insert(edges.end(), pedges.begin(), pedges.end());
  }
  return std::vector(edges.begin(), edges.end());
}

std::size_t PathVector::point_count() const
{
  return std::accumulate(cbegin(m_paths), cend(m_paths), 0, [](std::size_t n, const auto& path) {
    return n + path->points().size();
  });
}

std::deque<Path*> PathVector::paths() const
{
  return util::transform(m_paths, std::mem_fn(&std::unique_ptr<Path>::get));
}

Path* PathVector::find_path(const PathPoint& point) const
{
  for (auto&& path : m_paths) {
    if (path->contains(point)) {
      return path.get();
    }
  }
  return nullptr;
}

Path& PathVector::add_path(std::unique_ptr<Path> path)
{
  path->set_path_vector(this);
  return *m_paths.emplace_back(std::move(path));
}

Path& PathVector::add_path()
{
  return add_path(std::make_unique<Path>(this));
}

std::unique_ptr<Path> PathVector::remove_path(const Path &path)
{
  const auto it = std::find_if(m_paths.begin(), m_paths.end(), [&path](const auto& s_ptr) {
    return &path == s_ptr.get();
  });
  std::unique_ptr<Path> extracted_path;
  std::swap(extracted_path, *it);
  m_paths.erase(it);
  return extracted_path;
}

std::shared_ptr<PathPoint> PathVector::share(const PathPoint& path_point) const
{
  for (const auto& path : m_paths) {
    if (const auto& a = path->share(path_point); a != nullptr) {
      return a;
    }
  }
  return {};
}

std::set<PathPoint*> PathVector::points() const
{
  std::set<PathPoint*> points;
  for (const auto& path : m_paths) {
    const auto& ps = path->points();
    points.insert(ps.begin(), ps.end());
  }
  return points;
}

std::set<PathPoint*> PathVector::selected_points() const
{
  return util::remove_if(points(), [](const auto& p) { return !p->is_selected(); });
}

void PathVector::deselect_all_points() const
{
  for (auto* point : points()) {
    point->set_selected(false);
  }
}

void PathVector::draw_point_ids(QPainter& painter) const
{
  for (const auto* point : points()) {
    static constexpr QPointF offset{10.0, 10.0};
    painter.drawText(point->geometry().position().to_pointf() + offset, point->debug_id());
  }
}

PathObject* PathVector::path_object() const
{
  return m_path_object;
}

std::unique_ptr<PathVector> PathVector::join(const std::deque<PathVector*>& pvs, double eps)
{
  auto joined = std::make_unique<PathVector>();
  std::map<const PathPoint*, PathPoint*> point_mapping;
  for (const auto& pv : pvs) {
    auto pv_mapping = joined->copy_from(*pv).points;
    point_mapping.merge(pv_mapping);
  }

  for (const std::vector<PathPoint*>& correspondences : PathVectorIsomorphism(pvs).correspondences()) {
    std::set<PathPoint*> close_points;
    for (std::size_t i = 0; i < correspondences.size(); ++i) {
      for (std::size_t j = 0; j < i; ++j) {

      }
    }
  }

  std::deque<std::pair<const PathPoint*, const PathPoint*>> close_points;
  const auto eps2 = eps * eps;
  for (std::size_t i1 = 0; i1 < pvs.size(); ++i1) {
    for (std::size_t i2 = 0; i2 < i1; ++i2) {
      for (const auto* const p1 : pvs.at(i1)->points()) {
        for (const auto* const p2 : pvs.at(i2)->points()) {
          if ((p1->geometry().position() - p2->geometry().position()).euclidean_norm2() < eps2) {
            close_points.emplace_back(p1, p2);
          }
        }
      }
    }
  }

  for (std::size_t i = 0; i < close_points.size(); ++i) {
    const auto& [p1, p2] = close_points.at(i);
    PathPoint*& j1 = point_mapping.at(p1);
    PathPoint*& j2 = point_mapping.at(p2);
    auto* joined_point = joined->join({j1, j2});
    j1 = joined_point;
    j2 = joined_point;
  }

  return joined;
}

PathPoint* PathVector::join(std::set<PathPoint*> ps)
{
  if (ps.empty()) {
    return {};
  }

  std::shared_ptr<PathPoint> special;
  const auto replace_maybe = [&ps, &special](std::shared_ptr<PathPoint>& candidate) {
    if (ps.contains(candidate.get())) {
      if (!special) {
        special = candidate;
      } else {
        for (const auto& [key, tangent] : candidate->geometry().tangents()) {
          special->geometry().set_tangent(key, tangent);
        }
        candidate = special;
      }
    }
  };
  for (auto& path : m_paths) {
    for (auto* edge : path->edges()) {
      replace_maybe(edge->a());
      replace_maybe(edge->b());
    }
  }

  return special.get();
}

PathVector::Mapping PathVector::copy_from(const PathVector& other)
{
  std::map<const Path*, Path*> paths_map;
  for (const auto* other_path : other.paths()) {
    auto& path = add_path();
    paths_map.try_emplace(other_path, &path);
  }

  std::map<const omm::PathPoint*, std::shared_ptr<PathPoint>> points_map;
  for (const auto* const point : other.points()) {
    auto geometry = point->geometry();
    geometry.replace_tangents_key(paths_map);
    points_map.try_emplace(point, std::make_shared<PathPoint>(geometry, this));
  }

  for (const auto* other_path : other.paths()) {
    const auto other_edges = other_path->edges();
    auto& path = *paths_map.at(other_path);
    if (other_edges.empty()) {
      if (other_path->last_point()) {
        path.set_single_point(points_map.at(other_path->last_point().get()));
      }
    } else {
      for (const auto* other_edge : other_edges) {
        const auto& a = points_map.at(other_edge->a().get());
        const auto& b = points_map.at(other_edge->b().get());
        path.add_edge(std::make_unique<Edge>(a, b, &path));
      }
    }
  }

  std::map<const PathPoint*, PathPoint*> points_map_raw;
  for (const auto& [key, shared_ptr] : points_map) {
    points_map_raw.try_emplace(key, shared_ptr.get());
  }

  return {points_map_raw, paths_map};
}

QString PathVector::to_dot() const
{
  QString s;
  QTextStream ts(&s, QIODevice::WriteOnly);
  ts << "graph {\n";
  for (const auto* const edge : edges()) {
    ts << "\"";
    ts << edge->a()->debug_id();
    ts << "\" -- \"";
    ts << edge->b()->debug_id();
    ts << "\" [label=\"" << edge->label() << "\"];\n";
  }
  ts << "}";
  return s;
}

void PathVector::to_svg(const QString& filename) const
{
  const auto bb = bounding_box();
  QSvgGenerator svg;
  svg.setFileName(filename);
  const double size = std::max(bb.height(), bb.width());
  const QPointF margin(size / 3.0, size / 3.0);
  svg.setViewBox(QRectF(bb.topLeft() - margin, bb.bottomRight() + margin));
  QPainter painter(&svg);

  static constexpr auto colors = std::array{
      Qt::red,
      Qt::green,
      Qt::blue,
      Qt::cyan,
      Qt::magenta,
      Qt::yellow,
      Qt::gray,
      Qt::darkRed,
      Qt::darkGreen,
      Qt::darkBlue,
      Qt::darkCyan,
      Qt::darkMagenta,
      Qt::darkYellow,
      Qt::darkGray,
      Qt::lightGray,
  };

  std::map<const Path*, Qt::GlobalColor> path_colors;
  std::size_t path_index = 0;
  auto pen = painter.pen();
  auto font = painter.font();
  font.setPointSizeF(size / 10.0);
  painter.setFont(font);
  pen.setCosmetic(true);
  for (const auto* const path : paths()) {
    const auto color = colors.at(path_index % colors.size());
    pen.setColor(color);
    painter.setPen(pen);
    painter.drawPath(path->to_painter_path());
    path_colors.emplace(path, color);
    path_index += 1;
  }

  pen.setStyle(Qt::DotLine);
  for (const auto* const p : points()) {
    const auto p0 = p->geometry().position().to_pointf();
    for (const auto& [key, pc] : p->geometry().tangents()) {
      pen.setColor(path_colors.at(key.path));
      painter.setPen(pen);
      const auto pt = p->geometry().tangent_position(key).to_pointf();
      const auto start = key.direction == Direction::Forward ? pt : p0;
      const auto end = key.direction == Direction::Forward ? p0 : pt;
      QPainterPath path;
      path.moveTo(start);
      path.lineTo(end);
      if (!path.isEmpty()) {
        painter.drawPath(path);
      }
    }
    pen.setColor(Qt::black);
    pen.setWidthF(size / 100.0);
    painter.setPen(pen);
    painter.drawPoint(p0);

    pen.setWidthF(1.0);
    painter.setPen(pen);
    painter.drawText(p0, QString("%1").arg(p->debug_id()));
  }
}

QRectF PathVector::bounding_box() const
{
  static constexpr auto get_bb = [](const auto* const p) { return p->geometry().bounding_box(); };
  const auto bbs = util::transform<std::list>(points(), get_bb);
  const auto tls = util::transform(bbs, &QRectF::topLeft);
  const auto brs = util::transform(bbs, &QRectF::bottomRight);
  static constexpr auto cmp_x = [](const auto& a, const auto& b) { return a.x() < b.x(); };
  static constexpr auto cmp_y = [](const auto& a, const auto& b) { return a.y() < b.y(); };

  const QPointF tl(std::min_element(tls.begin(), tls.end(), cmp_x)->x(),
                   std::min_element(tls.begin(), tls.end(), cmp_y)->y());
  const QPointF br(std::max_element(brs.begin(), brs.end(), cmp_x)->x(),
                   std::max_element(brs.begin(), brs.end(), cmp_y)->y());
  return {tl, br};
}

}  // namespace omm
