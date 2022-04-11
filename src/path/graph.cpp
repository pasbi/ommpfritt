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
  [[maybe_unused, nodiscard]] Vertex& data(VertexDescriptor vertex);
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

std::set<Face> Graph::compute_faces() const
{
  std::set<Face> faces;


  if (!faces.empty()) {
    // we don't want to include the largest face, which is contains the whole universe expect the path.
    const auto it = std::max_element(faces.begin(), faces.end(), [](const auto& a, const auto& b) {
      return a.compute_aabb_area() < b.compute_aabb_area();
    });
    faces.erase(it);
  }

  return faces;
}

void Graph::Impl::add_vertex(PathPoint* path_point)
{
  (void) path_point;
}

bool Graph::Impl::add_edge(PathPoint* a, PathPoint* b)
{
  (void) a;
  (void) b;
  return  false;
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
  (void) edge;
  (void) vertex;
  return PolarCoordinates{};
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
  return dot;
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
