#pragma once

namespace omm
{

class Scene;

class SceneObjectModel // : public Gtk::TreeModel::ColumnRecord
{
public:
  explicit SceneObjectModel(Scene& scene);
  ~SceneObjectModel();

private:
  // Gtk::TreeModelColumn<Glib::ustring> m_column_label;
  Scene& m_scene;

};

}  // namespace omm