//#pragma once

//#include "tools/selectpointstool.h"

//namespace omm
//{
//class SelectSimilarTool : public SelectPointsBaseTool

//{
//public:
//  explicit SelectSimilarTool(Scene& scene);
//  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "SelectSimilarTool");
//  QString type() const override
//  {
//    return TYPE;
//  }
//  SceneMode scene_mode() const override
//  {
//    return SceneMode::Vertex;
//  }
//  void reset() override;

//private:
//  void on_property_value_changed(Property* property) override;
//  void update_selection();
//  void apply();
//  void update_base_selection();
////  bool is_similar(const PathIterator& a, const PathIterator& b) const;
////  std::set<PathIterator> m_base_selection;
//  void end() override;
//  void start() override;
//  void update_property_appearance();
//};

//}  // namespace omm
