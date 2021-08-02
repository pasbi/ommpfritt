#include "animation/animator.h"
#include "enumnames.h"
#include "logging.h"
#include "mainwindow/application.h"
#include "mainwindow/exporter.h"
#include "objects/view.h"
#include "scene/scene.h"
#include "subcommandlineparser.h"
#include "tags/tag.h"
#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <iomanip>
#include <iostream>
#include <memory>

namespace
{
void exit(omm::ExitStatus status)
{
  ::exit(static_cast<int>(status));
}

}  // namespace

template<typename T> T& find(omm::Scene& scene, const QString& name)
{
  const auto type_matches = ::filter_if(scene.object_tree().items(), [](const auto* c) {
    return c->type() == T::TYPE || std::is_same_v<T, omm::Object>;
  });
  const auto name_type_matches
      = ::filter_if(type_matches, [name](const auto* c) { return c->name() == name; });
  if (name_type_matches.empty()) {
    LERROR << QString("%1 '%2' not found.").arg(T::TYPE).arg(name);
    const QStringList view_names
        = ::transform<QString, QList>(type_matches, [](const auto* v) { return v->name(); });
    LINFO << QString("There are %1 objects of type [%2] in this scene:\n%3")
                 .arg(view_names.size())
                 .arg(T::TYPE)
                 .arg(view_names.join("\n"));
    exit(omm::ExitStatus::object_type_not_found);
  } else if (name_type_matches.size() > 1) {
    LWARNING << QString("%1 '%2' is ambiguous (%3) occurences.")
                    .arg(T::TYPE)
                    .arg(name)
                    .arg(name_type_matches.size());
  }
  return static_cast<omm::View&>(**name_type_matches.begin());
}

QString interpolate_filename(QString fn_template, int i)
{
  const int first_match = fn_template.indexOf(omm::SubcommandLineParser::FRAMENUMBER_PLACEHOLDER);
  if (first_match == -1) {
    return fn_template;
  }

  const int last_match
      = fn_template.lastIndexOf(omm::SubcommandLineParser::FRAMENUMBER_PLACEHOLDER);
  const QString placeholder = fn_template.mid(first_match, last_match - first_match + 1);
  if (placeholder.count(omm::SubcommandLineParser::FRAMENUMBER_PLACEHOLDER) != placeholder.size()) {
    LERROR << QObject::tr("Framenumber placeholder must be contiguous.");
    exit(omm::ExitStatus::invalid_input_format);
  }

  const auto formatted_number = QString("%1").arg(i, placeholder.size(), 10, QChar('0'));
  fn_template.replace(first_match, placeholder.size(), formatted_number);
  return fn_template;
}

void print_tree(const omm::Object& root, const QString& prefix = "")
{
  const auto tags = ::transform<QString, QList>(root.tags.items(), [](const omm::Tag* tag) {
                      return tag->type();
                    }).join(", ");
  const auto label = QString("%1[%2] (%3)").arg(root.type(), root.name(), tags);
  std::cout << prefix.toStdString() << label.toStdString() << "\n";

  for (const omm::Object* c : root.tree_children()) {
    print_tree(*c, prefix + " ");
  }
}

void prepare_scene(omm::Scene& scene, const std::set<omm::Object*>& visible_objects)
{
  for (auto* other : scene.object_tree().items()) {
    const auto is_descendant_of
        = [&other](const auto* object) { return object->is_ancestor_of(*other); };
    if (std::none_of(visible_objects.begin(), visible_objects.end(), is_descendant_of)) {
      other->property(omm::Object::VISIBILITY_PROPERTY_KEY)->set(omm::Object::Visibility::Hidden);
    }
  }
  for (auto* object : visible_objects) {
    object->property(omm::Object::VISIBILITY_PROPERTY_KEY)->set(omm::Object::Visibility::Visible);
  }
}

void prepare_scene(omm::Scene& scene, const omm::SubcommandLineParser& args)
{
  const QString& object_name = args.get<QString>("object-name", "");
  const QString& object_path = args.get<QString>("object-path", "");
  if (!object_name.isEmpty() && !object_path.isEmpty()) {
    LERROR << "options object-path and object-name are mutual exclusive.";
    exit(omm::ExitStatus::invalid_input_format);
  }

  const auto predicate = [object_name, object_path](const auto* object) {
    if (!object_name.isEmpty()) {
      return QRegularExpression(object_name).match(object->name()).hasMatch();
    } else if (!object_path.isEmpty()) {
      return QRegularExpression(object_path).match(object->tree_path()).hasMatch();
    } else {
      return true;
    }
  };

  const auto visible_objects = ::filter_if(scene.object_tree().items(), predicate);
  if (args.isSet("unique") && visible_objects.size() != 1) {
    LERROR << "Expected exactly one matching object but found " << visible_objects.size() << ".";
    if (visible_objects.empty()) {
      exit(omm::ExitStatus::object_name_not_found);
    } else {
      exit(omm::ExitStatus::non_unique_object_reference);
    }
  }
  prepare_scene(scene, visible_objects);
}

QSize calculate_resolution(int width, const omm::View& view)
{
  const auto size = view.property(omm::View::SIZE_PROPERTY_KEY)->value<omm::Vec2f>();
  return QSize(width, static_cast<int>(width / size.x * size.y));
}

void render(omm::Application& app, const omm::SubcommandLineParser& args)
{
  const auto scene_filename = args.get<QString>("input");
  const auto fn_template = args.get<QString>("output");
  if (!app.scene.load_from(scene_filename)) {
    exit(EXIT_FAILURE);
  }
  prepare_scene(app.scene, args);
  const int start_frame = args.get<int>("start-frame", 1);
  const int n_frames = args.get<int>("sequence-length", 1);
  auto& view = find<omm::View>(app.scene, args.get<QString>("view"));
  const bool force = args.isSet("overwrite");
  const auto resolution = calculate_resolution(args.get<int>("width"), view);

  const auto render = [&view, resolution, fn_template, force](omm::Animator& animator) {
    const QString filename = interpolate_filename(fn_template, animator.current());
    if (QFileInfo::exists(filename) && !force) {
      LERROR << QObject::tr("Refuse to overwrite existing file '%1'.").arg(filename);
      exit(omm::ExitStatus::refuse_overwrite_file);
    }
    omm::Exporter exporter{animator.scene};
    exporter.export_options.view = &view;
    if (filename.endsWith(".svg", Qt::CaseInsensitive)) {
      exporter.export_options.scale = omm::Exporter::SVG_SCALE_FACTOR;
      exporter.save_as_svg(filename);
    } else {
      exporter.export_options.x_resolution = resolution.width();
      exporter.y_resolution = resolution.height();
      exporter.save_as_raster(filename);
    }
  };

  auto& animator = app.scene.animator();
  animator.set_current(start_frame);
  render(animator);
  for (int i = 0; i < n_frames; ++i) {
    animator.advance();
    render(animator);
  }
}

void tree(omm::Application& app, const omm::SubcommandLineParser& args)
{
  const auto scene_filename = args.get<QString>("input");
  if (!app.scene.load_from(scene_filename)) {
    exit(EXIT_FAILURE);
  }
  print_tree(app.scene.object_tree().root());
}

std::unique_ptr<omm::Options> make_options(const omm::SubcommandLineParser& args)
{
  const auto have_opengl = [&args]() {
    if (args.command() == omm::SubcommandLineParser::COMMAND_RENDER) {
      return !args.isSet("no-opengl");
    } else {
      // other commands don't require opengl.
      return false;
    }
  }();
  return std::make_unique<omm::Options>(true, have_opengl);
}

const bool print_long_message = true;

// The logfile must be non-const and globally accessible, because the message handler lambda
// must not capture anything (see qInstallMessageHandler).
QFile logfile;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

// The level must be non-const and globally accessible, because the message handler lambda
// must not capture anything (see qInstallMessageHandler).
QString level = "";  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

int main(int argc, char* argv[])
{
  using namespace omm;
  QGuiApplication app(argc, argv);  // QCoreApplication does not provide font metrics
  SubcommandLineParser args(QApplication::arguments());

  level = args.get<QString>("verbosity", "warning");
  if (!::contains(omm::LogLevel::loglevels, level)) {
    const auto levels = ::transform<QString, QList>(::get_keys(omm::LogLevel::loglevels));
    std::cerr << "Unknown log level '" << level << "'. Use " << levels.join("|") << ".\n";
    std::cerr << std::flush;
    exit(omm::ExitStatus::invalid_input_format);
  }

  setup_logfile(logfile);
  qInstallMessageHandler([](QtMsgType type, const QMessageLogContext& ctx, const QString& msg) {
    handle_log(logfile, level, print_long_message, type, ctx, msg);
  });

  Application omm(app, make_options(args));

  using subcommand_t = std::function<void(Application&, const SubcommandLineParser&)>;
  static const std::map<QString, subcommand_t> f_map{
      {SubcommandLineParser::COMMAND_RENDER, &render},
      {SubcommandLineParser::COMMAND_TREE, &tree},
  };

  if (const auto it = f_map.find(args.command()); it == f_map.end()) {
    return EXIT_FAILURE;
  } else {
    it->second(omm, args);
    return EXIT_SUCCESS;
  }
}
