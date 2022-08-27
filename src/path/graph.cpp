//#include "path/graph.h"
//#include "path/pathpoint.h"
//#include "path/face.h"
//#include "path/edge.h"
//#include "path/pathvector.h"
//#include "path/path.h"
//#include "path/pathvectorview.h"
//#include <boost/graph/adjacency_list.hpp>
//#include <boost/graph/planar_face_traversal.hpp>
//#include <boost/graph/biconnected_components.hpp>
//#include <boost/property_map/property_map.hpp>

//namespace
//{

//omm::PolarCoordinates get_direction_at(const omm::Edge& edge, const omm::PathPoint& vertex)
//{
//  const auto d = &vertex == edge.a().get() ? omm::Point::Direction::Forward : omm::Point::Direction::Backward;
//  const auto& tangent = vertex.geometry().tangent({edge.path(), d});
//  static constexpr auto eps = 1e-2;
//  if (tangent.magnitude < eps) {
//    return omm::PolarCoordinates(edge.b()->geometry().position() - edge.a()->geometry().position());
//  } else {
//    return tangent;
//  }
//}

//}  // namespace

//namespace omm
//{

//struct edge_data_t
//{
//  using kind = boost::edge_property_tag;
//};

//struct vertex_data_t
//{
//  using kind = boost::vertex_property_tag;
//};


//class Graph::Impl
//{
//  using VertexProperty = boost::property<boost::vertex_index_t, std::size_t,
//                         boost::property<vertex_data_t, PathPoint*>>;
//  using EdgeProperty = boost::property<boost::edge_index_t, std::size_t,
//                       boost::property<edge_data_t, Edge*>>;
//private:
//  friend class Graph;
//  using G = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
//                                  VertexProperty, EdgeProperty>;
//  using VertexDescriptor = boost::graph_traits<G>::vertex_descriptor;
//  using EdgeDescriptor = boost::graph_traits<G>::edge_descriptor;
//  G m_g;
//  using Embedding = std::vector<std::deque<EdgeDescriptor>>;
//  [[nodiscard]] Embedding compute_embedding() const;
//public:
//  Impl(const PathVector& path_vector);
//};

//Graph::Graph(const PathVector& path_vector)
//    : m_impl(std::make_unique<Impl>(path_vector))
//{
//}

//std::set<Face> Graph::compute_faces() const
//{
//  class Visitor : public boost::planar_face_traversal_visitor
//  {
//  public:
//    Visitor(const Impl& impl, std::set<Face>& faces) : m_faces(faces), m_impl(impl) {}
//    void begin_face()
//    {
//      m_edges.clear();
//    }

//    void next_edge(const Impl::EdgeDescriptor& edge)
//    {
//      m_edges.emplace_back(boost::get(edge_data_t{}, m_impl.m_g)[edge]);
//    }

//    void end_face()
//    {
//      m_faces.emplace(PathVectorView(std::move(m_edges)));
//    }

//  private:
//    std::set<Face>& m_faces;
//    const Impl& m_impl;
//    std::deque<Edge*> m_edges;
//  };

//  const auto embedding = m_impl->compute_embedding();
//  std::set<Face> faces;
//  Visitor visitor(*m_impl, faces);
//  auto emap = boost::make_iterator_property_map(embedding.begin(), get(boost::vertex_index, m_impl->m_g));
//  boost::planar_face_traversal(m_impl->m_g, emap, visitor);

//  if (faces.empty()) {
//    return {};
//  }

////  // we don't want to include the largest face, which is contains the whole universe except the
////  // interior of the path vector.
////  const auto it = std::max_element(faces.begin(), faces.end(), [](const auto& a, const auto& b) {
////    return a.compute_aabb_area() < b.compute_aabb_area();
////  });
////  faces.erase(it);

//  return faces;
//}

//Graph::~Graph() = default;

//Graph::Impl::Embedding Graph::Impl::compute_embedding() const
//{
//  const auto n = boost::num_vertices(m_g);
//  Embedding embedding(n);
//  const auto& vertex_property_map = boost::get(vertex_data_t{}, m_g);
//  const auto& edge_property_map = boost::get(edge_data_t{}, m_g);
//  for (auto [v, vend] = boost::vertices(m_g); v != vend; ++v) {
//    std::deque<EdgeDescriptor> edges;
//    for (auto [e, eend] = out_edges(*v, m_g); e != eend; ++e) {
//      edges.emplace_back(*e);
//    }
//    const auto arg = [&edge_property_map, &hinge=*vertex_property_map[*v]](const EdgeDescriptor& ed) {
//      return python_like_mod(get_direction_at(*edge_property_map[ed], hinge).argument, 2 * M_PI);
//    };
//    const auto cw = [&arg](const EdgeDescriptor ed1, const EdgeDescriptor ed2) {
//      return arg(ed1) > arg(ed2);
//    };
//    std::sort(edges.begin(), edges.end(), cw);
//    embedding[*v] = std::move(edges);
//  }

//  std::cout << "embedding:\n";
//  for (const auto& es : embedding) {
//    std::cout << "===\n";
//    for (const auto& e : es) {
//      std::cout << "  " << boost::get(edge_data_t{}, m_g)[e]->label().toStdString() << "\n";
//    }
//  }
//  std::cout << std::endl;

//  return embedding;
//}

//Graph::Impl::Impl(const PathVector& path_vector)
//{
//  std::map<const PathPoint*, VertexDescriptor> vertex_map;
//  auto vertex_property_map = boost::get(vertex_data_t{}, m_g);
//  auto edge_property_map = boost::get(edge_data_t{}, m_g);
//  for (auto* const p : path_vector.points()) {
//    const auto vertex_descriptor = boost::add_vertex(m_g);
//    vertex_map[p] = vertex_descriptor;
//    boost::put(vertex_property_map, vertex_descriptor, p);
//  }
//  for (const auto* path : path_vector.paths()) {
//    for (auto* const edge : path->edges()) {
//      const auto u = vertex_map.at(edge->a().get());
//      const auto v = vertex_map.at(edge->b().get());
//      const auto& [edge_descriptor, success] = boost::add_edge(u, v, m_g);
//      boost::put(edge_property_map, edge_descriptor, edge);
//      assert(success);
//    }
//  }
//}

////void Graph::remove_articulation_edges() const
////{
//////  auto components = get(boost::edge_index, *m_impl);
//////  const auto n = boost::biconnected_components(*m_impl, components);
//////  LINFO << "Found " << n << " biconnected components.";
////  std::set<std::size_t> art_points;
////  boost::articulation_points(*m_impl, std::inserter(art_points, art_points.end()));
////  const auto edge_between_articulation_points = [&art_points, this](const Impl::EdgeDescriptor& e) {
////    const auto o = [&art_points, this](const Impl::VertexDescriptor& v) {
////      return degree(v, *m_impl) <= 1 || art_points.contains(v);
////    };
////    return o(e.m_source) && o(e.m_target);
////  };
////  boost::remove_edge_if(edge_between_articulation_points, *m_impl);
////}

//}  // namespace omm
