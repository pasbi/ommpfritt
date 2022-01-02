#include "path/graph.h"
#include "path/pathpoint.h"
#include "path/face.h"
#include "path/edge.h"
#include "path/pathvector.h"
#include "path/path.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/planar_face_traversal.hpp>
#include <boost/graph/biconnected_components.hpp>
#include <boost/property_map/property_map.hpp>

namespace
{

struct Vertex
{
  ::transparent_set<omm::PathPoint*> points;
  [[nodiscard]] QString debug_id() const { return (*points.begin())->debug_id(); }
};

template<typename Graph> void identify_edges(Graph& graph)
{
  auto e_index = get(boost::edge_index, graph);
  typename boost::graph_traits<Graph>::edges_size_type edge_count = 0;
  for(auto [it, end] = edges(graph); it != end; ++it) {
    put(e_index, *it, edge_count++);
  }
}

}  // namespace

namespace omm
{

struct edge_data_t
{
  using kind = boost::edge_property_tag;
};

struct vertex_data_t
{
  using kind = boost::vertex_property_tag;
};

using namespace boost;
class Graph::Impl : public adjacency_list<vecS,
                                          vecS,
                                          undirectedS,
                                          property<vertex_index_t, std::size_t,
                                           property<vertex_data_t, Vertex>>,
                                          property<edge_index_t, std::size_t,
                                           property<edge_data_t, Edge>>
                                         >
{
public:
  using Joint = ::transparent_set<PathPoint*>;
  using VertexIndexMap = std::map<const PathPoint*, int>;
  using JointMap = std::deque<Joint>;
  using EdgeDescriptor = boost::detail::edge_desc_impl<boost::undirected_tag, std::size_t>;
  using VertexDescriptor = std::size_t;
  using Embedding = std::vector<std::deque<EdgeDescriptor>>;
  using adjacency_list::adjacency_list;
  [[nodiscard]] const Vertex& data(VertexDescriptor vertex) const;
  [[nodiscard]] Vertex& data(VertexDescriptor vertex);
  [[nodiscard]] const Edge& data(EdgeDescriptor edge_descriptor) const;
  [[nodiscard]] Edge& data(EdgeDescriptor edge_descriptor);
  void add_vertex(PathPoint* path_point);
  bool add_edge(PathPoint* a, PathPoint* b);
  [[nodiscard]] VertexDescriptor lookup_vertex(const PathPoint* p) const;
  [[nodiscard]] Embedding compute_embedding() const;
  [[nodiscard]] PolarCoordinates get_direction_at(const Edge& edge, VertexDescriptor vertex) const;

private:
  VertexIndexMap m_vertex_index_map;
  JointMap m_joint_map;
};

Graph::Graph(const PathVector& path_vector)
  : m_impl(std::make_unique<Impl>())
{
  for (const auto* path : path_vector.paths()) {
    PathPoint* last_path_point = nullptr;
    for (auto* point : path->points()) {
      m_impl->add_vertex(point);
      if (last_path_point != nullptr) {
        m_impl->add_edge(point, last_path_point);
      }
      last_path_point = point;
    }
  }
}

std::vector<Face> Graph::compute_faces() const
{
  using Faces =  std::list<Face>;
  Faces faces;
  struct Visitor : boost::planar_face_traversal_visitor
  {
    Visitor(const Impl& impl, Faces& faces) : faces(faces), m_impl(impl) {}
    Faces& faces;
    std::optional<Face> current_face;

    void begin_face()
    {
      current_face = Face{};
    }

    void next_edge(const Impl::EdgeDescriptor edge)
    {
      [[maybe_unused]] bool success = current_face->add_edge(m_impl.data(edge));
      assert(success);
    }

    void end_face()
    {
      faces.emplace_back(*current_face);
      current_face = std::nullopt;
    }

  private:
    const Impl& m_impl;
  };

  identify_edges(*m_impl);
  const auto embedding = m_impl->compute_embedding();
  Visitor visitor{*m_impl, faces};
  boost::planar_face_traversal(*m_impl, &embedding[0], visitor);

  if (faces.empty()) {
    return {};
  }

  // we don't want to include the largest face, which is contains the whole universe expect the path.
  const auto areas = util::transform(faces, std::mem_fn(&Face::compute_aabb_area));
  const auto largest_face_i = std::distance(areas.begin(), std::max_element(areas.begin(), areas.end()));
  faces.erase(std::next(faces.begin(), largest_face_i));

  // NOLINTNEXTLINE(modernize-return-braced-init-list)
  std::vector vfaces(faces.begin(), faces.end());
  vfaces.erase(std::unique(vfaces.begin(), vfaces.end()), vfaces.end());
  return vfaces;
}

void Graph::Impl::add_vertex(PathPoint* path_point)
{
  // if a point is not joined, we need a set containing only that lonely point.
  const auto vertex_points = [path_point]() -> ::transparent_set<PathPoint*> {
    if (auto set = path_point->joined_points(); set.empty()) {
      return {path_point};
    } else {
      return set;
    }
  }();

  // if a vertex was already assigned to a joined point, re-use that vertex.
  for (auto* jp : vertex_points) {
    if (const auto it = m_vertex_index_map.find(jp); it != m_vertex_index_map.end()) {
      m_vertex_index_map.emplace(path_point, it->second);
      return;
    }
  }

  // if none of this point's joints has a vertex assigned, create a new one.
  const auto n = boost::add_vertex(*this);
  m_vertex_index_map.emplace(path_point, n);
  m_joint_map.emplace_back(vertex_points);
  data(lookup_vertex(path_point)).points = vertex_points;
}

bool Graph::Impl::add_edge(PathPoint* a, PathPoint* b)
{
  assert(&a->path() == &b->path());
  const auto ai = m_vertex_index_map.at(a);
  const auto bi = m_vertex_index_map.at(b);
  const auto [edge, was_inserted] = boost::add_edge(ai, bi, *this);
  auto& edge_data = data(edge);
  edge_data.a = a;
  edge_data.b = b;
  return was_inserted;
}

Graph::Impl::VertexDescriptor Graph::Impl::lookup_vertex(const PathPoint* p) const
{
  return m_vertex_index_map.at(p);
}

Graph::Impl::Embedding Graph::Impl::compute_embedding() const
{
  const auto n = boost::num_vertices(*this);
  Graph::Impl::Embedding embedding(n);
  for (auto [v, vend] = boost::vertices(*this); v != vend; ++v) {
    std::deque<EdgeDescriptor> edges;
    for (auto [e, eend] = out_edges(*v, *this); e != eend; ++e) {
      edges.emplace_back(*e);
    }
    std::sort(edges.begin(), edges.end(), [this, v=*v](const auto e1, const auto e2) {
      const auto a1 = python_like_mod(get_direction_at(data(e1), v).argument, 2 * M_PI);
      const auto a2 = python_like_mod(get_direction_at(data(e2), v).argument, 2 * M_PI);
      return a1 < a2;
    });
    embedding[*v] = std::move(edges);
  }
  return embedding;
}

PolarCoordinates Graph::Impl::get_direction_at(const Edge& edge, VertexDescriptor vertex) const
{
  const auto compute_edge_direction = [](const Point& major, const Point& minor, const auto& get_tangent) {
    static constexpr double eps = 0.00001;
    if (const auto tangent = get_tangent(major); tangent.magnitude > eps) {
      return tangent;
    } else {
      return PolarCoordinates{-major.position() + minor.position()};
    }
  };
  const auto& joint = m_joint_map.at(vertex);
  if (joint.contains(edge.a)) {
    return compute_edge_direction(edge.a->geometry(), edge.b->geometry(), std::mem_fn(&Point::left_tangent));
  } else {
    assert(joint.contains(edge.b));
    return compute_edge_direction(edge.b->geometry(), edge.a->geometry(), std::mem_fn(&Point::right_tangent));
  }
}

Graph::~Graph() = default;

const Vertex& Graph::Impl::data(const VertexDescriptor vertex) const
{
  return get(vertex_data_t{}, *this)[vertex];
}

Vertex& Graph::Impl::data(const VertexDescriptor vertex)
{
  return get(vertex_data_t{}, *this)[vertex];
}

const Edge& Graph::Impl::data(EdgeDescriptor edge) const
{
  return get(edge_data_t{}, *this)[edge];
}

Edge& Graph::Impl::data(EdgeDescriptor edge)
{
  return get(edge_data_t{}, *this)[edge];
}

QString Graph::to_dot() const
{
  const auto& g = *m_impl;
  QString dot = "graph x {\n";
  for (auto ep = boost::edges(g); ep.first != ep.second; ++ep.first) {
    const auto& edge = *ep.first;
    auto v1 = boost::source(edge, g);
    auto v2 = boost::target(edge, g);
    dot += "\"";
    dot += g.data(v1).debug_id();
    dot += "\" -- \"";
    dot += g.data(v2).debug_id();
    dot += "\" [label=\"";
    dot += g.data(edge).label();
    dot += "\"];\n";
  }
  dot += "}";
  return dot;\
}

void Graph::remove_articulation_edges() const
{
//  auto components = get(boost::edge_index, *m_impl);
//  const auto n = boost::biconnected_components(*m_impl, components);
//  LINFO << "Found " << n << " biconnected components.";
  std::set<std::size_t> art_points;
  boost::articulation_points(*m_impl, std::inserter(art_points, art_points.end()));
  const auto edge_between_articulation_points = [&art_points, this](const Impl::edge_descriptor& e) {
    const auto o = [&art_points, this](const Impl::vertex_descriptor& v) {
      return degree(v, *m_impl) <= 1 || art_points.contains(v);
    };
    return o(e.m_source) && o(e.m_target);
  };
  boost::remove_edge_if(edge_between_articulation_points, *m_impl);
}

}  // namespace omm
