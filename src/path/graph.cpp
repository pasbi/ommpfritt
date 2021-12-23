#include "path/graph.h"
#include "path/pathpoint.h"
#include "path/face.h"
#include "path/edge.h"
#include "path/pathvector.h"
#include "path/path.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/planar_face_traversal.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>

namespace
{

struct Vertex
{
  std::set<omm::PathPoint*> points;
  QString debug_id() const { return (*points.begin())->debug_id(); }
};

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
  using Joint = std::set<PathPoint*>;
  using VertexIndexMap = std::map<const PathPoint*, int>;
  using JointMap = std::deque<Joint>;
  using EdgeDescriptor = boost::detail::edge_desc_impl<boost::undirected_tag, std::size_t>;
  using VertexDescriptor = std::size_t;
  using adjacency_list::adjacency_list;
  const Vertex& data(VertexDescriptor vertex) const;
  Vertex& data(VertexDescriptor vertex);
  const Edge& data(EdgeDescriptor edge_descriptor) const;
  Edge& data(EdgeDescriptor edge_descriptor);
  void add_vertex(PathPoint* path_point);
  bool add_edge(PathPoint* a, PathPoint* b);
  VertexDescriptor lookup_vertex(const PathPoint* p) const;

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
  identify_edges();
}

void Graph::identify_edges()
{
  using Impl = Graph::Impl;
  auto e_index = get(edge_index, *m_impl);
  graph_traits<Impl>::edges_size_type edge_count = 0;
  for(auto [it, end] = edges(*m_impl); it != end; ++it) {
    put(e_index, *it, edge_count++);
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

  typedef std::vector< std::vector< graph_traits<Graph::Impl>::edge_descriptor > >
      embedding_storage_t;
  typedef boost::iterator_property_map
      < embedding_storage_t::iterator,
        property_map<Graph::Impl, vertex_index_t>::type
       >
          embedding_t;

  embedding_storage_t embedding_storage(num_vertices(*m_impl));
  embedding_t embedding(embedding_storage.begin(), get(vertex_index, *m_impl));
  boyer_myrvold_planarity_test(boyer_myrvold_params::graph = *m_impl,
                               boyer_myrvold_params::embedding = &embedding[0]);
//  const auto embedding = impl().compute_embedding();
  Visitor visitor{*m_impl, faces};
  boost::planar_face_traversal(*m_impl, &embedding[0], visitor);

  if (faces.empty()) {
    return {};
  }

  // we don't want to include the largest face, which is contains the whole universe expect the path.
  const auto areas = ::transform<double>(faces, std::mem_fn(&Face::compute_aabb_area));
  const auto largest_face_i = std::distance(areas.begin(), std::max_element(areas.begin(), areas.end()));
  faces.erase(std::next(faces.begin(), largest_face_i));
  return std::vector(faces.begin(), faces.end());
}

void Graph::Impl::add_vertex(PathPoint* path_point)
{
  // if a point is not joined, we need a set containing only that lonely point.
  const auto vertex_points = [path_point]() {
    if (const auto set = path_point->joined_points(); set.empty()) {
      return std::set{path_point};
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

std::string Graph::to_dot() const
{
  const auto& g = *m_impl;
  std::ostringstream os;
  os << "graph x {\n";
  for (auto ep = boost::edges(g); ep.first != ep.second; ++ep.first) {
    const auto& edge = *ep.first;
    auto v1 = boost::source(edge, g);
    auto v2 = boost::target(edge, g);
    os << "\""
       << g.data(v1).debug_id().toStdString()
       << "\" -- \""
       << g.data(v2).debug_id().toStdString()
       << "\" [label=\""
       << g.data(edge).label().toStdString()
       << "\"];\n";
  }
  os << "}";
  return os.str();
}

}  // namespace omm