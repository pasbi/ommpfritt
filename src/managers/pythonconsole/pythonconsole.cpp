#include "managers/pythonconsole/pythonconsole.h"
#include <QTextEdit>
#include <QPushButton>
#include <QEvent>
#include <QKeyEvent>
#include "widgets/codeedit.h"
#include <pybind11/embed.h>
#include "scene/scene.h"
#include "python/pythonengine.h"
#include "python/scenewrapper.h"
#include "widgets/referencelineedit.h"
#include "mainwindow/application.h"

namespace omm
{

PythonConsole::PythonConsole(Scene& scene)
  : Manager(QObject::tr("Styles", "PythonConsole"), scene, make_menu_bar())
{
  setWindowTitle(QObject::tr("PythonConsole", "Python Console"));
  setObjectName(TYPE);

  auto widget = std::make_unique<QWidget>();
  auto layout = std::make_unique<QVBoxLayout>(widget.get());
  m_layout = layout.get();
  layout.release(); // ownership is managed by Qt

  auto header_layout = std::make_unique<QHBoxLayout>().release();
  m_layout->addLayout(header_layout);

  auto ref_filter_widget = std::make_unique<ReferenceLineEdit>(scene, [](auto*){});
  ref_filter_widget->set_filter(AbstractPropertyOwner::Flag::HasScript);
  m_associated_item_widget = ref_filter_widget.get();
  m_associated_item_widget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  header_layout->addWidget(ref_filter_widget.release());

  auto exec_button = std::make_unique<QPushButton>(QObject::tr("run", "PythonConsole"));
  connect(exec_button.get(), &QPushButton::clicked, this, &PythonConsole::eval);
  exec_button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  header_layout->addWidget(exec_button.release());

  auto clear_button = std::make_unique<QPushButton>(QObject::tr("clear", "PythonConsole"));
  connect(clear_button.get(), &QPushButton::clicked, this, &PythonConsole::clear);
  clear_button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  header_layout->addWidget(clear_button.release());

  auto output = std::make_unique<CodeEdit>();
  m_output = output.get();
  m_output->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_output->set_placeholder_text(QObject::tr("no output yet.", "PythonConsole").toStdString());
  m_output->set_editable(false);
  m_layout->addWidget(output.release());

  auto commandline = std::make_unique<CodeEdit>();
  m_commandline = commandline.get();
  m_commandline->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  m_commandline->set_placeholder_text( QObject::tr("enter command ...", "PythonConsole")
                                        .toStdString() );
  m_commandline->installEventFilter(this);
  m_commandline->set_caption_modifiers(caption_modifiers);
  m_layout->addWidget(commandline.release());

  m_layout->setContentsMargins(0, 0, 0, 0);
  set_widget(std::move(widget));

  scene.python_engine.Observed<PythonIOObserver>::register_observer(*this);
}

PythonConsole::~PythonConsole()
{
  scene().python_engine.Observed<PythonIOObserver>::unregister_observer(*this);
}

std::unique_ptr<QMenuBar> PythonConsole::make_menu_bar()
{
  auto menu_bar = std::make_unique<QMenuBar>();

  // auto py_menu = menu_bar->addMenu(tr("Python");
  // connect(py_menu->addAction(tr("run")), &QAction::triggered, this, &PythonConsole::run);

  return menu_bar;
}

void PythonConsole::eval()
{
  const auto code = m_commandline->code();
  push_command(code);
  m_output->put(QObject::tr(">>> ", "PythonConsole").toStdString() + code, CodeEdit::Stream::stdout);

  using namespace pybind11::literals;
  const auto locals = pybind11::dict("scene"_a=SceneWrapper(scene()));
  auto result = scene().python_engine.eval(code, locals, nullptr);
  if (!result.is_none()) {
    scene().python_engine.eval( "print(result)",
                                pybind11::dict("result"_a=result), nullptr);
  }

  m_commandline->clear();
}

void PythonConsole::on_stdout(const void* associated_item, const std::string& text)
{
  if (accept(associated_item)) { m_output->put(text, CodeEdit::Stream::stdout); }
}

void PythonConsole::on_stderr(const void* associated_item, const std::string& text)
{
  if (accept(associated_item)) { m_output->put(text, CodeEdit::Stream::stderr); }
}

bool PythonConsole::accept(const void* associated_item) const
{
  if (associated_item == nullptr) {
    return true;
  }
  return associated_item == m_associated_item_widget->value();
}

void PythonConsole::clear()
{
  m_output->clear();
}

bool PythonConsole::eventFilter(QObject* object, QEvent* event)
{
  if (object == m_commandline) {
    if (event->type() == QEvent::KeyPress) {
      auto* key_event = static_cast<QKeyEvent*>(event);
      if (!(key_event->modifiers() & caption_modifiers)) {
        switch (key_event->key()) {
        case Qt::Key_Return:
            eval();
            key_event->accept();
            return true;
          break;
        case Qt::Key_Up:
          get_previous_command();
          key_event->accept();
          return true;
        case Qt::Key_Down:
          get_next_command();
          key_event->accept();
          return true;
        }
      }
    }
  }

  return Manager::eventFilter(object, event);
}

void PythonConsole::get_previous_command()
{
  m_commandline->clear();
  if (m_command_stack_pointer != m_command_stack.begin()) {
    std::advance(m_command_stack_pointer, -1);
    m_commandline->set_code(*m_command_stack_pointer);
  } else {
  }
}

void PythonConsole::get_next_command()
{
  m_commandline->clear();
  if (m_command_stack_pointer != m_command_stack.end()) {
    m_commandline->set_code(*m_command_stack_pointer);
    std::advance(m_command_stack_pointer, 1);
  } else {
  }
}

void PythonConsole::push_command(const std::string& command)
{
  m_command_stack.erase(m_command_stack_pointer, m_command_stack.end());
  m_command_stack.push_back(command);
  m_command_stack_pointer = m_command_stack.end();
}

void PythonConsole::keyPressEvent(QKeyEvent* event)
{
  if (!Application::instance().key_bindings.call(*event, *this)) {
    Manager::keyPressEvent(event);
  }
}

std::map<std::string, QKeySequence> PythonConsole::default_bindings()
{
  return {
    { QT_TRANSLATE_NOOP("PythonConsole", "clear python console"),
      QKeySequence(QObject::tr("Ctrl+K", "PythonConsole")) }
  };
}

void PythonConsole::call(const std::string& command)
{
  dispatch(command, {
    { "clear python console", [this](){ clear();  } },
  });
}

void PythonConsole::populate_menu(QMenu& menu)
{
  auto& key_bindings = Application::instance().key_bindings;
  menu.addAction(key_bindings.make_action(*this, "clear python console").release());
}

}  // namespace omm
