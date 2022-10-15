#include "path/graph.h"
#include "path/edge.h"
#include "path/pathvector.h"
#include "removeif.h"
#include "transform.h"
#include <numeric>

namespace omm
{

Graph::Graph(const PathVector& path_vector) : m_edges(util::transform<std::set>(path_vector.edges()))
{
  for (auto* edge : m_edges) {
    m_adjacent_edges[edge->a().get()].insert(edge);
    m_adjacent_edges[edge->b().get()].insert(edge);
  }
}

void Graph::remove_edge(Edge* edge)
{
  m_edges.erase(edge);

  for (auto* const p : edge->points()) {
    const auto it = m_adjacent_edges.find(p);
    it->second.erase(edge);
    if (it->second.empty()) {
      m_adjacent_edges.erase(it);
    }
  }
}

const std::set<Edge*>& Graph::edges() const
{
  return m_edges;
}

const std::set<Edge*>& Graph::adjacent_edges(const PathPoint& p) const
{
  return m_adjacent_edges.at(&p);
}

void Graph::remove_dead_ends()
{
  const auto is_not_on_fringe = [this](const auto& edge) { return degree(*edge->a()) != 1 && degree(*edge->b()) != 1; };
  std::set<Edge*> fringe = util::remove_if(m_edges, is_not_on_fringe);

  const auto add_to_fringe = [this, &fringe, &is_not_on_fringe](const PathPoint& p) {
    const auto edges = util::remove_if(m_adjacent_edges.at(&p), is_not_on_fringe);
    fringe.insert(edges.begin(), edges.end());
  };

  while (!fringe.empty()) {
    auto* const current = fringe.extract(fringe.begin()).value();
    remove_edge(*current);
    const auto i1 = m_adjacent_edges.find(current->a().get());
    const auto i2 = m_adjacent_edges.find(current->b().get());
    if (i1 != m_adjacent_edges.end() && i2 != m_adjacent_edges.end()) {
      // current was not on fringe, impossible.
    } else if (i1 != m_adjacent_edges.end()) {
      add_to_fringe(*current->a());
    } else if (i2 != m_adjacent_edges.end()) {
      add_to_fringe(*current->b());
    } else {
      // may happen if current edge is lonely
    }
  }
}

std::list<Graph> Graph::connected_components() const
{
  auto unvisited_vs = vertices();
  std::list<std::set<PathPoint*>> components;
  while (!unvisited_vs.empty()) {
    const auto& current_component = components.emplace_back(connected_component(**unvisited_vs.begin()));
    for (auto* const v : current_component) {
      unvisited_vs.erase(v);
    }
  }

  return util::transform(components, [this](const auto& component) { return subgraph(component); });
}

Graph Graph::subgraph(const std::set<PathPoint*>& vertices) const
{
  Graph g;
  for (auto* const v : vertices) {
    const auto edges = m_adjacent_edges.at(v);
    g.m_adjacent_edges.emplace(v, edges);
    g.m_edges.insert(edges.begin(), edges.end());
  }
  return g;
}

std::set<PathPoint*> Graph::connected_component(PathPoint& seed) const
{
  std::set dfs{&seed};
  std::set<PathPoint*> component;
  while (!dfs.empty()) {
    auto* const v = dfs.extract(dfs.begin()).value();
    component.insert(v);
    for (auto* const e : m_adjacent_edges.at(v)) {
      for (auto* const p : e->points()) {
        if (!component.contains(p)) {
          dfs.insert(p);
        }
      }
    }
  }
  return component;
}

std::set<PathPoint*> Graph::vertices() const
{
  std::set<PathPoint*> vs;
  for (const auto* const edge : m_edges) {
    vs.insert(edge->a().get());
    vs.insert(edge->b().get());
  }
  return vs;
}

std::size_t Graph::degree(const PathPoint& p) const
{
  const auto it = m_adjacent_edges.find(&p);
  if (it == m_adjacent_edges.end()) {
    return 0;
  } else {
    const auto& edges = it->second;
    const auto accumulate_degrees = [p = &p](const auto accu, const Edge* const edge) {
      return accu + (edge->a().get() == p ? 1 : 0) + (edge->b().get() == p ? 1 : 0);
    };
    return std::accumulate(edges.begin(), edges.end(), 0, accumulate_degrees);
  }
}

void Graph::remove_edge(Edge& edge)
{
  m_edges.erase(&edge);
  for (const auto& p : {edge.a(), edge.b()}) {
    if (const auto it = m_adjacent_edges.find(p.get()); it != m_adjacent_edges.end()) {
      it->second.erase(&edge);
      if (it->second.empty()) {
        m_adjacent_edges.erase(it);
      }
    }
  }
}

}  // namespace omm
