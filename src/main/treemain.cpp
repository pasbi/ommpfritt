#include "main/commandlineparser.h"
#include "main/application.h"
#include "tags/tag.h"
#include "objects/object.h"
#include "scene/scene.h"

namespace
{

void print_tree(const omm::Object& root, const QString& prefix = "")
{
  const auto tags = util::transform<QList>(root.tags.items(), [](const omm::Tag* tag) {
                      return tag->type();
                    }).join(", ");
  const auto label = QString("%1[%2] (%3)").arg(root.type(), root.name(), tags);
  std::cout << prefix.toStdString() << label.toStdString() << "\n";

  for (const omm::Object* c : root.tree_children()) {
    print_tree(*c, prefix + " ");
  }
}

}  // namespace

namespace omm
{

int tree_main(const CommandLineParser& args, Application& app)
{
  const auto fn = args.scene_filename();
  if (fn.isEmpty()) {
    LFATAL("No scene file name given.");
  }
  if (!app.scene->load_from(fn)) {
    LFATAL("Failed to load %s.", fn.toUtf8().data());
  }
  ::print_tree(app.scene->object_tree().root());

  return EXIT_SUCCESS;
}

}  // namespace omm
