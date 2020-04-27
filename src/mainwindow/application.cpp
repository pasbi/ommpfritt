#include "application.h"

#include <QSettings>
#include <cassert>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QAbstractButton>

#include "tags/scripttag.h"
#include "tags/nodestag.h"

#include "mainwindow/toolbar/toolbar.h"
#include "mainwindow/mainwindow.h"
#include "managers/manager.h"
#include "scene/scene.h"
#include "commands/addcommand.h"
#include "mainwindow/actions.h"
#include "objects/object.h"
#include "tools/toolbox.h"
#include "tags/tag.h"
#include "mainwindow/viewport/viewport.h"
#include "mainwindow/exportdialog.h"
#include "logging.h"
#include "widgets/pointdialog.h"
#include "commands/movecommand.h"
#include "scene/history/historymodel.h"
#include "scene/stylelist.h"
#include "managers/manager.h"
#include "preferences/preferencedialog.h"
#include <QTranslator>
#include "registers.h"

namespace
{
constexpr auto FILE_ENDING = ".omm";

auto load_translator(const QString& prefix, const QLocale& locale)
{
  auto translator = std::make_unique<QTranslator>();
  const auto locale_name = locale.name().toStdString();
  if (translator->load(prefix + "_" + locale.name(), ":/qm", "_", ".qm")) {
    LINFO << "Installing translator '" << prefix << "' for " << locale_name << ".";
    return translator;
  } else {
    LWARNING << "No translator '" << prefix << "' found for " << locale_name
             << ". Using fallback-translator.";
    if (translator->load(prefix + "_", ":/qm", "_", ".qm")) {
      LINFO << "Installing fallback-translator.";
      return translator;
    } else {
      LERROR << "failed to load fallback-translator.";
      return std::unique_ptr<QTranslator>(nullptr);
    }
  }
}

int img_mean(const QImage& image)
{
  double w = 0.0;
  double sum = 0.0;
  for (int y = 0; y < image.height(); ++y) {
    assert(image.format() == QImage::Format_ARGB32_Premultiplied);
    const QRgb* rgba_line = reinterpret_cast<const QRgb*>(image.scanLine(y));
    for (int x = 0; x < image.width(); ++x) {
      const auto& rgba = rgba_line[x];
      const double alpha = qAlpha(rgba) / 255.0;
      sum += qRed(rgba) * alpha;
      sum += qGreen(rgba) * alpha;
      sum += qBlue(rgba) * alpha;
      w += 3.0 * alpha;
    }
  }
  return sum / w;
}

QKeySequence push_back(const QKeySequence& s, int t)
{
  switch (s.count()) {
  case 0:
    return QKeySequence(t);
  case 1:
    return QKeySequence(s[0], t);
  case 2:
    return QKeySequence(s[0], s[1], t);
  case 3:
    return QKeySequence(s[0], s[1], s[2], t);
  case 4:
    return QKeySequence(s[1], s[2], s[3], t);
  default:
    Q_UNREACHABLE();
    return QKeySequence();
  }
}

auto load_locale()
{
  const auto locale = QSettings().value(omm::MainWindow::LOCALE_SETTINGS_KEY).toLocale();
  return locale;
}

void init()
{
  omm::register_everything();
  QCoreApplication::setOrganizationName(QObject::tr("omm"));
  QCoreApplication::setApplicationName(::QApplication::translate("QObject", "ommpfritt"));
}

QString scene_directory_hint(const QString& scene_filename)
{
  if (QFileInfo::exists(scene_filename)) {
    return QFileInfo(scene_filename).dir().path();
  } else {
    return QDir::homePath();
  }
}

}  // namespace

namespace omm
{

const std::set<int> Application::keyboard_modifiers { Qt::Key_Shift, Qt::Key_Control, Qt::Key_Alt,
                                                      Qt::Key_Meta };
Application* Application::m_instance = nullptr;

Application::Application(QCoreApplication& app, std::unique_ptr<Options> options)
  : scene((init(), python_engine))
  , m_app(app)
  , m_options(std::move(options))
  , m_locale(load_locale())
{
  if (m_instance == nullptr) {
    m_instance = this;
  } else {
    LFATAL("Resetting application instance.");
  }

  scene.set_selection({});
  if (!this->options().is_cli) {
    ui_colors.apply();
    m_reset_keysequence_timer.setSingleShot(true);
    m_reset_keysequence_timer.setInterval(1000);
    connect(&m_reset_keysequence_timer, &QTimer::timeout, this, [this]() {
      m_pending_key_sequence = QKeySequence();
    });
  }

  install_translators();
  scene.polish();
}

Application::~Application()
{
}

Application& Application::instance()
{
  assert(m_instance != nullptr);
  return *m_instance;
}

void Application::set_main_window(MainWindow& main_window)
{
  m_main_window = &main_window;
}

void Application::evaluate() const
{
  for (Tag* tag : scene.tags()) {
    tag->evaluate();
  }
}

void Application::quit()
{
  if (can_close()) {
    LINFO << "Quit application.";
    m_app.quit();
  } else {
    LINFO << "Aborted quit.";
  }
}

void Application::update_undo_redo_enabled()
{
}

bool Application::can_close()
{
  if (scene.history().has_pending_changes()) {
    const auto decision =
      QMessageBox::question( m_main_window,
                             tr("Question."),
                             tr("Some pending changes will be lost if you don't save."),
                             QMessageBox::Close | QMessageBox::Cancel | QMessageBox::Save,
                             QMessageBox::Save );
    switch (decision) {
    case QMessageBox::Close: return true;
    case QMessageBox::Cancel: return false;
    case QMessageBox::Save: return save();
    default:
      qCritical() << "Unexpected response code: " << decision;
      return false;
    }
  } else {
    return true;
  }
}

bool Application::save(const QString& filename)
{
  if (!scene.save_as(filename)) {
    LWARNING << "Error saving scene as '" << filename << "'.";
    QMessageBox::critical( m_main_window,
                           tr("Error."),
                           tr("The scene could not be saved at '%1'.").arg(filename),
                           QMessageBox::Ok, QMessageBox::Ok );
    return false;
  } else {
    return true;
  }
}

bool Application::save()
{
  const QString filename = scene.filename();
  if (filename.isEmpty()) {
    return save_as();
  } else {
    return save(filename);
  }
}

bool Application::save_as()
{
  LINFO << m_main_window;
  QFileDialog dialog(m_main_window);
  dialog.setWindowTitle(tr("Save scene as ..."));
  dialog.setDirectoryUrl(scene_directory_hint(scene.filename()));
  dialog.setDefaultSuffix(FILE_ENDING);
  if (dialog.exec() == QDialog::Accepted) {
    const auto files = dialog.selectedFiles();
    assert(files.size() == 1);
    return scene.save_as(files.front());
  } else {
    return false;
  }
}

void Application::reset()
{
  if (!can_close()) { return; }

  LINFO << "reset scene.";
  scene.set_selection({});
  QTimer::singleShot(0, &scene, SLOT(reset()));
}

void Application::load(const QString& filename, bool force)
{
  if (force || can_close()) {
    scene.set_selection({});
    QTimer::singleShot(0, [this, filename]() {
      if (!scene.load_from(filename)) {
        QMessageBox::critical( m_main_window,
                               tr("Error."),
                               tr("Loading scene from '%1' failed.").arg(filename),
                               QMessageBox::Ok );
      }
    });
  }
}

void Application::load()
{
  if (can_close()) {
    const QString filename =
      QFileDialog::getOpenFileName(m_main_window,
                                   tr("Load scene ..."),
                                   scene_directory_hint(scene.filename()));
    if (filename.isEmpty()) {
      return;
    }

    load(filename, true);
  }
}

bool Application::perform_action(const QString& action_name)
{
  if (action_name == "undo") {
    scene.history().undo();
  } else if (action_name == "redo") {
    scene.history().redo();
  } else if (action_name == "remove selection") {
    scene.remove(main_window(), scene.selection());
  } else if (action_name == "new document") {
    reset();
  } else if (action_name == "save document") {
    save();
  } else if (action_name == "save document as") {
    save_as();
  } else if (action_name == "load document") {
    load();
  } else if (action_name == "export") {
    static ExportDialog* export_dialog = nullptr;
    if (export_dialog == nullptr) {
      export_dialog = new omm::ExportDialog(scene, main_window());
    }
    export_dialog->exec();
  } else if (action_name == "make smooth") {
    actions::make_smooth(*this);
  } else if (action_name == "make linear") {
    actions::make_linear(*this);
  } else if (action_name == "remove points") {
    actions::remove_selected_points(*this);
  } else if (action_name == "subdivide") {
    actions::subdivide(*this);
  } else if (action_name == "evaluate") {
    evaluate();
  } else if (action_name == "restore default layout") {
    main_window()->restore_default_layout();
  } else if (action_name == "save layout ...") {
    main_window()->save_layout();
  } else if (action_name == "load layout ...") {
    main_window()->load_layout();
  } else if (action_name == "new toolbar") {
    spawn_toolbar();
  } else if (action_name == "switch between object and point selection") {
    scene.tool_box().switch_between_object_and_point_selection();
  } else if (action_name == "previous tool") {
    scene.tool_box().set_previous_tool();
  } else if (action_name == "select all") {
    actions::select_all(*this);
  } else if (action_name == "deselect all") {
    actions::deselect_all(*this);
  } else if (action_name == "invert selection") {
    actions::invert_selection(*this);
  } else if (action_name == "new style") {
      using command_type = AddCommand<List<Style>>;
      auto style = scene.default_style().clone();
      assert(style->scene() == &scene);
      scene.submit<command_type>(scene.styles(), std::move(style));
  } else if (action_name == "convert objects") {
    actions::convert_objects(*this);
  } else if (action_name == "reset viewport") {
    main_window()->viewport().reset();
  } else if (action_name == "show point dialog") {
    const auto paths = Object::cast<Path>(scene.item_selection<Object>());
    if (paths.size() > 0) {
      PointDialog(paths, main_window()).exec();
    }
  } else if (action_name == "preferences") {
    PreferenceDialog().exec();
  } else {
    for (const auto& key : Object::keys()) {
      if (key == action_name) {
        const auto modifiers = QApplication::keyboardModifiers();
        if (modifiers & Qt::ControlModifier) {
          insert_object(key, InsertionMode::AsChild);
        } else if (modifiers & Qt::ShiftModifier) {
          insert_object(key, InsertionMode::AsParent);
        } else {
          insert_object(key, InsertionMode::Default);
        }
        return true;
      }
    }
    for (const auto& key : Manager::keys()) {
      if (key == action_name) {
        spawn_manager(key);
        return true;
      }
    }
    for (const auto& key : Tool::keys()) {
      if (key == action_name) {
        scene.tool_box().set_active_tool(key);
        return true;
      }
    }
    for (const auto& key : Tag::keys()) {
      if (key == action_name) {
        const auto object_selection = scene.item_selection<Object>();
        if (!object_selection.empty()) {
          auto macro = scene.history().start_macro(tr("Add Tag"));
          for (auto&& object : object_selection) {
            using AddTagCommand = omm::AddCommand<omm::List<omm::Tag>>;
            scene.submit<AddTagCommand>(object->tags, Tag::make(key, *object));
          }
        }
        return true;
      }
    }
    return false;
  }
  return true;
}

bool Application::dispatch_key(int key, Qt::KeyboardModifiers modifiers, CommandInterface& ci)
{
  const auto dispatch_sequence = [this](CommandInterface& ci) {
    LINFO << m_pending_key_sequence;
    const auto action_name = key_bindings.find_action(ci.type(), m_pending_key_sequence);
    if (!action_name.isEmpty() && ci.perform_action(action_name)) {
      m_pending_key_sequence = QKeySequence();
      return true;
    } else {
      return false;
    }
  };

  m_pending_key_sequence = push_back(m_pending_key_sequence, key | modifiers);
  m_reset_keysequence_timer.start();
  if (dispatch_sequence(ci)) {
    return true;
  } else if (&ci != &Application::instance()) {
    return dispatch_sequence(Application::instance());
  } else {
    return false;
  }
}

bool Application::dispatch_key(int key, Qt::KeyboardModifiers modifiers)
{
  const QPoint mouse_position = QCursor::pos();
  for (Manager* manager : m_managers) {
    const QRect manager_rect(manager->mapToGlobal(manager->rect().topLeft()),
                             manager->mapToGlobal(manager->rect().bottomRight()));
    if (manager_rect.contains(mouse_position)) {
      return dispatch_key(key, modifiers, *manager);
    }
  }
  return dispatch_key(key, modifiers, Application::instance());
}

QString Application::type() const { return TYPE; }

MessageBox &Application::message_box()
{
  return scene.message_box();
}

MainWindow* Application::main_window() const { return m_main_window; }

Object& Application::insert_object(const QString &key, InsertionMode mode)
{
  const auto translated_key =QApplication::translate("any-context", key.toUtf8().constData());
  auto macro = scene.history().start_macro(tr("Create %1").arg(translated_key));
  using add_command_type = AddCommand<ObjectTree>;
  auto object = Object::make(key, &scene);
  object->set_object_tree(scene.object_tree());
  auto& ref = *object;

  Object* parent = nullptr;
  Object* predecessor = nullptr;
  std::vector<Object*> children;
  switch (mode) {
  case InsertionMode::AsChild:
    if (const auto selection = scene.item_selection<Object>(); selection.size() == 1) {
      parent = *selection.begin();
    }
    break;
  case InsertionMode::AsParent: {
    auto selection = scene.item_selection<Object>();
    Object::remove_internal_children(selection);
    children = Object::sort(selection);
    parent = children.empty() ? &scene.object_tree().root() : &children.back()->tree_parent();
    if (!children.empty()) {
      if (std::size_t pos = children.back()->position(); pos > 0) {
        predecessor = parent->tree_children()[pos-1];
      }
    }
    break;
  }
  default:
    break;
  }

  scene.submit<add_command_type>(scene.object_tree(), std::move(object));
  ref.set_global_transformation(ObjectTransformation(), Space::Scene);
  using move_command_t = MoveCommand<ObjectTree>;
  using move_context_t = move_command_t::context_type;
  if (!children.empty()) {
    const auto move_contextes = ::transform<move_context_t>(children, [&ref](auto* c) {
      return move_context_t(*c, ref, nullptr);
    });
    scene.submit<move_command_t>(scene.object_tree(), move_contextes);
    move_context_t move_context(ref, *parent, predecessor);
    if (move_context.is_strictly_valid(scene.object_tree())) {
      // the move will fail if the object is already at the correct position.
      scene.submit<move_command_t>(scene.object_tree(), std::vector { move_context });
    }
  } else if (parent != nullptr) {
    const move_context_t move_context(ref, *parent, nullptr);
    scene.submit<move_command_t>(scene.object_tree(), std::vector { move_context });
  }

  ref.post_create_hook();
  return ref;
}
void Application::register_auto_invert_icon_button(QAbstractButton& button)
{
  const auto update_button_icon = [&button]() {
    const QColor text_color = qApp->palette().color(QPalette::Active, QPalette::ButtonText);
    QImage img = button.icon().pixmap(QSize(1024, 1024)).toImage();
    if (std::abs(img_mean(img) - text_color.value()) > 100) {
      img.invertPixels(QImage::InvertRgb);
      QSignalBlocker blocker(&button);
      button.setIcon(QIcon(QPixmap::fromImage(img)));
    }
  };

  const auto connection = connect(qApp, &QApplication::paletteChanged, update_button_icon);
  connect(&button, &QObject::destroyed, this, [connection]() {
    disconnect(connection);
  });
  update_button_icon();
}

Manager& Application::spawn_manager(const QString& type)
{
  auto manager = Manager::make(type, scene);
  main_window()->assign_unique_objectname(*manager);
  auto& ref = *manager;
  main_window()->addDockWidget(Qt::TopDockWidgetArea, manager.release());
  ref.setFloating(true);
  return ref;
}

ToolBar& Application::spawn_toolbar()
{
  auto toolbar = std::make_unique<ToolBar>();
  main_window()->assign_unique_objectname(*toolbar);
  auto& ref = *toolbar;
  main_window()->addToolBar(toolbar.release());
  return ref;
}

Manager& Application::get_active_manager(const QString& type)
{
  for (Manager* m : Application::instance().managers(type)) {
    if (m->is_visible() && !m->is_locked()) {
      return *m;
    }
  }
  return spawn_manager(type);
}

void Application::register_manager(Manager& manager)
{
  m_managers.insert(&manager);
}

void Application::unregister_manager(Manager& manager)
{
  m_managers.erase(&manager);
}

std::set<Manager*> Application::managers(const QString& type) const
{
  std::set<Manager*> managers;
  for (Manager* m : m_managers) {
    if (m->type() == type) {
      managers.insert(m);
    }
  }
  return managers;
}

const Preferences& preferences()
{
  return Application::instance().preferences;
}

void Application::install_translators()
{
  const auto qms = { "qtbase", "omm" };
  for (const QString& qm : qms) {
    auto translator = load_translator(qm, m_locale);
    if (translator) {
      m_app.installTranslator(translator.get());
      m_translators.insert(std::move(translator));
    };
  }
}
}  // namespace omm
