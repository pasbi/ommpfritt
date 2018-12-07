#include "commands/movecommand.h"
#include "objects/object.h"
#include "renderers/style.h"
#include "scene/scene.h"
#include "scene/tree.h"
#include "scene/list.h"

namespace
{

template<typename Structure>
using Context = typename omm::MoveCommand<Structure>::Context;


template<typename Structure>
auto make_old_contextes( const Structure& structure,
                         const std::vector<Context<Structure>>& new_contextes )
{
  const auto make_old_context = [&structure](const auto& new_context) {
    return Context<Structure>(new_context.subject, structure.predecessor(new_context.subject));
  };
  return ::transform<Context<Structure>>(new_contextes, make_old_context);
}

}  // namespace

namespace omm
{

template<typename Structure> MoveCommand<Structure>
::MoveCommand(Structure& structure, const std::vector<Context>& new_contextes)
  : Command(QObject::tr("reparent").toStdString())
  , m_old_contextes(make_old_contextes(structure, new_contextes))
  , m_new_contextes(new_contextes)
  , m_structure(structure)
{
}

template<typename Structure> void MoveCommand<Structure>::redo()
{
  for (auto& context : m_new_contextes) {
    m_structure.move(context);
  }
}

template<typename Structure> void MoveCommand<Structure>::undo()
{
  for (auto& context : m_old_contextes) {
    m_structure.move(context);
  }
}

template class MoveCommand<Tree<Object>>;
template class MoveCommand<List<Style>>;

}  // namespace omm
