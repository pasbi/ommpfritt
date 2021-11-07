//#include "commands/modifysegmentscommand.h"
//#include "scene/scene.h"
//#include "objects/path.h"
//#include "tools/toolbox.h"
//#include "tools/tool.h"

//namespace
//{
//using namespace omm;
//using Segments = ModifySegmentsCommand::Segments;
//auto get_old_segments(const Path& path, const Segments& new_segments)
//{
//  assert(new_segments.size() == path.segments.size());
//  Segments old_segments;
//  old_segments.reserve(new_segments.size());
//  for (std::size_t i = 0; i < new_segments.size(); ++i) {
//    if (new_segments[i]) {
//      old_segments.push_back(path.segments[i]);
//    } else {
//      old_segments.push_back({});
//    }
//  }
//  return old_segments;
//}

//}  // namespace

//namespace omm
//{
//ModifySegmentsCommand ::ModifySegmentsCommand(const QString& label,
//                                              Path& path,
//                                              const Segments& segments)
//    : Command(label), m_path(path), m_old_segments(get_old_segments(path, segments)),
//      m_new_segments(segments)
//{
//}

//void ModifySegmentsCommand::undo()
//{
//  apply(m_old_segments);
//}

//void ModifySegmentsCommand::redo()
//{
//  apply(m_new_segments);
//}

//void ModifySegmentsCommand::apply(const ModifySegmentsCommand::Segments& segments)
//{
//  assert(segments.size() == m_path.segments.size());
//  for (std::size_t i = 0; i < m_path.segments.size(); ++i) {
//    if (segments[i]) {
//      m_path.segments[i] = *segments[i];
//    }
//  }
//  m_path.update();
//  m_path.scene()->tool_box().active_tool().reset();
//}

//}  // namespace omm
