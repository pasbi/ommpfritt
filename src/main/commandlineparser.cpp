#include "main/commandlineparser.h"
#include "logging.h"
#include <QSize>
#include <iostream>

QList<QCommandLineOption> options() {
  using namespace omm;
  return {
    {
      {"m", CommandLineParser::MODE_KEY},
      QObject::tr("The mode (%1|%2|%3)").arg(CommandLineParser::GUI_MODE_NAME,
                                             CommandLineParser::RENDER_MODE_NAME,
                                             CommandLineParser::TREE_MODE_NAME),
      QObject::tr("MODE"),
      "gui"
    },
    {
      CommandLineParser::VERBOSITY_KEY,
      QObject::tr("The verbosity (fatal|critical|warning|info|debug)."),
      QObject::tr("LEVEL"),
      "warning"
    },
    {
      {"o", CommandLineParser::OUTPUT_KEY},
      QObject::tr("Where to save the batch renderings. Use `%1` as framenumber placeholder.")
         .arg(QString(omm::CommandLineParser::FRAMENUMBER_PLACEHOLDER).repeated(4)),
      QObject::tr("FILENAME")
    },
    {
      {"y", CommandLineParser::ALLOW_OVERWRITE_KEY},
      QObject::tr("Overwrite existing files without warning.")
    },
    {
      {"w", CommandLineParser::WIDTH_KEY},
      QObject::tr("The width of the rendering. "
                  "Height is calculated based on the aspect ratio of the selected view."),
      QObject::tr("W"),
      QString("1000")
    },
    {
      {"s", CommandLineParser::START_FRAME_KEY},
      QObject::tr("start-frame of the rendering (optional)."),
      QObject::tr("#FRAME"),
      "1"
    },
    {
      {"V", CommandLineParser::VIEW_NAME_KEY},
      QObject::tr("name of the view."),
      QObject::tr("NAME"),
    },
    {
      {"q", CommandLineParser::OBJECT_NAME_KEY},
      QObject::tr("The name of the object to render. "
                  "Keep clear to render the whole scene. "
                  "The name does not need to be unique, all matching objects are rendered. "
                  "Perl-like regular expressions are supported."),
      QObject::tr("NAME_REGEX"),
      "",
    },
    {
      {"u", CommandLineParser::UNIQUE_KEY},
      QObject::tr("If this option is set, the target object (see 'object-path' or "
                  "'object-name' argument) "
                  "must be unique, i.e., there must be exactly one such object. "
                  "The application terminates if this requirement is violated."),
    },
    {
      {"p", CommandLineParser::OBJECT_PATH_KEY},
      QObject::tr("The path of the object to render. "
                  "Keep clear to render the whole scene. "
                  "The path does not need to be unique, all matching objects are rendered. "
                  "Perl-like regular expressions are supported."),
      QObject::tr("PATH_REGEX"),
      "",
    },
    {
      {"n", CommandLineParser::SEQUENCE_LENGTH_KEY},
      QObject::tr("number of frames (optional)."),
      QObject::tr("#FRAMES"),
      "1"
    },
    {
      {"G", CommandLineParser::NO_OPENGL_KEY},
      QObject::tr("disable OpenGL.")
    }
  };
}

namespace omm
{
CommandLineParser::CommandLineParser(const QStringList& args)
{
  parser.addOptions(options());
  parser.addHelpOption();
  parser.addVersionOption();
  parser.process(args);
}

bool CommandLineParser::is_set(const QString& name) const
{
  return parser.isSet(name);
}

template<> QString CommandLineParser::get<QString>(const QString& name) const
{
  QString value = parser.value(name);
  if (value.isEmpty()) {
    LERROR << QString("<%1> must be specified.").arg(name);
    std::cout << parser.helpText().toStdString();
    exit(EXIT_FAILURE);
  }
  return value;
}

template<> int CommandLineParser::get<int>(const QString& name) const
{
  bool ok = true;
  const auto value = this->get<QString>(name);
  auto ivalue = value.toInt(&ok);
  if (!ok) {
    LERROR << QObject::tr("Expected integer argument for '%1', but got '%2'.").arg(name, value);
    exit(EXIT_FAILURE);
  }
  return ivalue;
}

QString omm::CommandLineParser::scene_filename() const
{
  const auto args = parser.positionalArguments();
  switch (args.size()) {
  case 0:
    LINFO << "No positional arguments given.";
    return {};
  case 1:
    LINFO << "Single positional argument given: " << args.front();
    return args.front();
  default:
    LFATAL("Multiple (%d) positional arguments given, which is not supported currently.", args.size());
    Q_UNREACHABLE();
  }
}

}  // namespace omm
