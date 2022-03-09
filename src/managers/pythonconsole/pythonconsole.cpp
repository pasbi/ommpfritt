#include "managers/pythonconsole/pythonconsole.h"
#include "main/application.h"
#include "managers/pythonconsole/pythonconsoletitlebar.h"
#include "python/pythonengine.h"
#include "python/scenewrapper.h"
#include "scene/scene.h"
#include "widgets/codeedit.h"
#include "widgets/referencelineedit.h"
#include <QCoreApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QPushButton>
#include <QTextEdit>

namespace omm
{
PythonConsole::PythonConsole(Scene& scene)
    : Manager(QCoreApplication::translate("any-context", "PythonConsole"), scene)
{
  setTitleBarWidget(std::make_unique<PythonConsoleTitleBar>(*this).release());

  auto widget = std::make_unique<QWidget>();
  auto layout = std::make_unique<QVBoxLayout>(widget.get());
  m_layout = layout.get();
  widget->setLayout(layout.release());

  auto* header_layout = std::make_unique<QHBoxLayout>().release();
  m_layout->addLayout(header_layout);

  auto ref_filter_widget = std::make_unique<ReferenceLineEdit>();
  ref_filter_widget->set_scene(scene);
  using Flag = Flag;
  ref_filter_widget->set_filter(PropertyFilter({{Flag::HasScript}, {Flag::HasPythonNodes}}));
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
  m_output->set_placeholder_text(QObject::tr("no output yet.", "PythonConsole"));
  m_output->set_editable(false);
  m_layout->addWidget(output.release());

  auto commandline = std::make_unique<CodeEdit>();
  m_commandline = commandline.get();
  m_commandline->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  m_commandline->set_placeholder_text(QObject::tr("enter command ...", "PythonConsole"));
  m_commandline->installEventFilter(this);
  m_commandline->set_caption_modifiers(caption_modifiers);
  m_layout->addWidget(commandline.release());

  m_layout->setContentsMargins(0, 0, 0, 0);
  set_widget(std::move(widget));

  connect(&PythonEngine::instance(), &PythonEngine::output, this, &PythonConsole::on_output);
  {
    using namespace pybind11::literals;
    m_locals = new pybind11::dict("scene"_a = SceneWrapper(scene));
  }
}

PythonConsole::~PythonConsole()
{
  delete static_cast<pybind11::dict*>(m_locals);
}

void PythonConsole::on_output(const void* associated_item,
                              const QString& text,
                              const Stream& stream)
{
  if (accept(associated_item)) {
    if (!text.isEmpty()) {
      m_output->put(text, stream);
      m_output->scroll_to_bottom();
    }
  }
}

void PythonConsole::eval()
{
  const auto code = m_commandline->code();
  push_command(code);
  m_output->put(QObject::tr(">>> ", "PythonConsole") + code, Stream::stdout_);

  PythonEngine::instance().exec(code, *static_cast<pybind11::dict*>(m_locals), nullptr);

  m_commandline->clear();
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
      auto* key_event = dynamic_cast<QKeyEvent*>(event);
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
  }
}

void PythonConsole::get_next_command()
{
  m_commandline->clear();
  if (m_command_stack_pointer != m_command_stack.end()) {
    m_commandline->set_code(*m_command_stack_pointer);
    std::advance(m_command_stack_pointer, 1);
  }
}

void PythonConsole::push_command(const QString& command)
{
  m_command_stack.erase(m_command_stack_pointer, m_command_stack.end());
  m_command_stack.push_back(command);
  m_command_stack_pointer = m_command_stack.end();
}

bool PythonConsole::perform_action(const QString& name)
{
  if (name == "clear console") {
    clear();
  } else if (name == "py-evaluate") {
    eval();
  } else {
    return false;
  }
  return true;
}

QString PythonConsole::type() const
{
  return TYPE;
}

}  // namespace omm
