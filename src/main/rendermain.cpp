#include <QFileInfo>
#include <QRegularExpression>

#include "commandlineparser.h"
#include "scene/scene.h"
#include "logging.h"
#include "objects/view.h"
#include "mainwindow/exporter.h"
#include "main/application.h"
#include "animation/animator.h"
#include "removeif.h"

namespace
{

using namespace omm;

template<typename T> T& find(Scene& scene, const QString& name)
{
  const auto type_matches = util::remove_if(scene.object_tree().items(), [](const auto* c) {
    return c->type() != T::TYPE && !std::is_same_v<T, Object>;
  });

  const auto name_type_matches = util::remove_if(type_matches, [name](const auto* c) {
    return c->name() != name;
  });

  if (name_type_matches.empty()) {
    const QStringList view_names = util::transform<QList>(type_matches, [](const auto* v) { return v->name(); });
    LINFO << QString("There are %1 objects of type [%2] in this scene:\n%3")
                 .arg(view_names.size())
                 .arg(T::TYPE)
                 .arg(view_names.join("\n"));
    LFATAL("%s '%s' not found.", T::TYPE, name.toUtf8().data());
  } else if (name_type_matches.size() > 1) {
    LWARNING << QString("%1 '%2' is ambiguous (%3) occurences.")
                    .arg(T::TYPE)
                    .arg(name)
                    .arg(name_type_matches.size());
  }
  return static_cast<View&>(**name_type_matches.begin());
}

QString interpolate_filename(QString fn_template, int i)
{
  const int first_match = fn_template.indexOf(CommandLineParser::FRAMENUMBER_PLACEHOLDER);
  if (first_match == -1) {
    return fn_template;
  }

  const int last_match
      = fn_template.lastIndexOf(CommandLineParser::FRAMENUMBER_PLACEHOLDER);
  const QString placeholder = fn_template.mid(first_match, last_match - first_match + 1);
  if (placeholder.count(CommandLineParser::FRAMENUMBER_PLACEHOLDER) != placeholder.size()) {
    LFATAL("Framenumber placeholder must be contiguous.");
  }

  const auto formatted_number = QString("%1").arg(i, placeholder.size(), 10, QChar('0'));
  fn_template.replace(first_match, placeholder.size(), formatted_number);
  return fn_template;
}

void prepare_scene(Scene& scene, const std::set<Object*>& visible_objects)
{
  for (auto* other : scene.object_tree().items()) {
    const auto is_descendant_of
        = [&other](const auto* object) { return object->is_ancestor_of(*other); };
    if (std::none_of(visible_objects.begin(), visible_objects.end(), is_descendant_of)) {
      other->property(Object::VISIBILITY_PROPERTY_KEY)->set(Object::Visibility::Hidden);
    }
  }
  for (auto* object : visible_objects) {
    object->property(Object::VISIBILITY_PROPERTY_KEY)->set(Object::Visibility::Visible);
  }
}

void prepare_scene(Scene& scene, const CommandLineParser& args)
{
  const QString& object_name = args.get<QString>(CommandLineParser::OBJECT_NAME_KEY, {});
  const QString& object_path = args.get<QString>(CommandLineParser::OBJECT_PATH_KEY, {});
  if (!object_name.isEmpty() && !object_path.isEmpty()) {
    LFATAL("options object-path and object-name are mutual exclusive.");
  }

  const auto is_not_visible = [object_name, object_path](const auto* object) {
    if (!object_name.isEmpty()) {
      return !QRegularExpression(object_name).match(object->name()).hasMatch();
    } else if (!object_path.isEmpty()) {
      return !QRegularExpression(object_path).match(object->tree_path()).hasMatch();
    } else {
      return false;
    }
  };

  const auto visible_objects = util::remove_if(scene.object_tree().items(), is_not_visible);
  if (args.is_set(CommandLineParser::UNIQUE_KEY) && visible_objects.size() != 1) {
    LFATAL("Expected exactly one matching object but found %d.", static_cast<int>(visible_objects.size()));
  }
  prepare_scene(scene, visible_objects);
}

QSize calculate_resolution(int width, const View& view)
{
  const auto size = view.property(View::SIZE_PROPERTY_KEY)->value<Vec2f>();
  return QSize(width, static_cast<int>(width / size.x * size.y));
}

}  // namespace

namespace omm
{

int render_main(const CommandLineParser& args, Application& app)
{
  if (const auto fn = args.scene_filename(); fn.isEmpty()) {
    LFATAL("No scene filename given.");
  } else if (!app.scene->load_from(fn)) {
    LFATAL("Failed to open %s.", fn.toUtf8().data());
  }

  const auto fn_template = args.get<QString>(CommandLineParser::OUTPUT_KEY);
  prepare_scene(*app.scene, args);
  const int start_frame = args.get<int>(CommandLineParser::START_FRAME_KEY);
  const int n_frames = args.get<int>(CommandLineParser::SEQUENCE_LENGTH_KEY);
  auto& view = find<View>(*app.scene, args.get<QString>(CommandLineParser::VIEW_NAME_KEY));
  const bool force = args.is_set(CommandLineParser::ALLOW_OVERWRITE_KEY);
  const auto resolution = calculate_resolution(args.get<int>(CommandLineParser::WIDTH_KEY), view);

  const auto render = [&view, resolution, fn_template, force](Animator& animator) {
    const QString filename = interpolate_filename(fn_template, animator.current());
    if (QFileInfo::exists(filename) && !force) {
      LFATAL("Refuse to overwrite existing file '%s'.", filename.toUtf8().data());
    }
    Exporter exporter{animator.scene};
    exporter.export_options.view = &view;
    if (filename.endsWith(".svg", Qt::CaseInsensitive)) {
      exporter.export_options.scale = Exporter::SVG_SCALE_FACTOR;
      exporter.save_as_svg(filename);
    } else {
      exporter.export_options.x_resolution = resolution.width();
      exporter.y_resolution = resolution.height();
      exporter.save_as_raster(filename);
    }
  };

  auto& animator = app.scene->animator();
  animator.set_current(start_frame);
  render(animator);
  for (int i = 0; i < n_frames; ++i) {
    animator.advance();
    render(animator);
  }

  return EXIT_SUCCESS;
}

}  // namespace omm
