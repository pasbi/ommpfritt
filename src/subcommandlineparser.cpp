#include "subcommandlineparser.h"
#include <iostream>
#include "logging.h"
#include <QSize>

static constexpr QSize DEFAULT_RESOLUTION(1000, 1000);
static constexpr auto RESOLUTION_SEPARATOR = 'x';

QCommandLineOption make_input_option()
{
  return  {
    { "f", "input" },
    QObject::tr("The input scene file."),
    QObject::tr("filename")
  };
}

static const std::map<QString, QList<QCommandLineOption>> options {
  {
    "render",
    {
      make_input_option(),
      {
        { "o", "output" },
        QObject::tr("Where to save the batch renderings. Use `%1` as framenumber placeholder.")
                      .arg(QString(omm::SubcommandLineParser::FRAMENUMBER_PLACEHOLDER).repeated(4)),
        QObject::tr("filename")
      },
      {
        { "y", "overwrite" },
        QObject::tr("Overwrite existing files without warning.")
      },
      {
        { "r", "resolution" },
        QObject::tr("Resolution of the rendering (optional)."),
        QObject::tr("W%1H").arg(RESOLUTION_SEPARATOR),
        QString("%1%2%3")
                      .arg(DEFAULT_RESOLUTION.width())
                      .arg(RESOLUTION_SEPARATOR)
                      .arg(DEFAULT_RESOLUTION.height())
      },
      {
        { "s", "start-frame" },
        QObject::tr("start-frame of the rendering (optional)."),
        QObject::tr("#FRAME"),
        "1"
      },
      {
        { "v", "view" },
        QObject::tr("name of the view."),
        QObject::tr("NAME"),
      },
      {
        { "q", "object" },
        QObject::tr("The name of the object to render. Keep clear to render the whole scene."),
        QObject::tr("NAME"),
        "",
      },
      {
        { "n", "sequence-length" },
        QObject::tr("number of frames (optional)."),
        QObject::tr("#FRAMES"),
        "1"
      },
    }
  },
  {
    "tree",
    {
      make_input_option(),
    }
  },
};

namespace omm
{

SubcommandLineParser::SubcommandLineParser(int argc, char* argv[])
  : m_appname(argv[0])
  , m_command(argc == 1 ? "" : argv[1])
{
  if (argc == 1 || m_command == "--help" || m_command == "-h") {
    print_help();
    exit(EXIT_SUCCESS);
  }

  const auto it = options.find(m_command);
  if (it == options.end()) {
    LERROR << QObject::tr("Unrecognized command: %1. Try '%2 --help' for more information.")
              .arg(m_command, m_appname);
    exit(EXIT_FAILURE);
  }

  addOptions(options.at(m_command));
  addHelpOption();

  QStringList args;
  for (int i = 0; i < argc; ++i) {
    if (i != 1) {
      args.append(argv[i]);
    }
  }
  process(args);
}

template<> QString SubcommandLineParser::get<QString>(const QString& name) const
{
  const QString value = this->value(name);
  if (value.isEmpty()) {
    LERROR << QString("<%1> must be specified.").arg(name);
    std::cout << helpText().toStdString();
    exit(EXIT_FAILURE);
  }
  return value;
}

template<> int SubcommandLineParser::get<int>(const QString& name) const
{
  bool ok = true;
  const auto value = this->get<QString>(name);
  const auto ivalue = value.toInt(&ok);
  if (!ok) {
    LERROR << QObject::tr("Expected integer argument for '%1', but got '%2'.").arg(name).arg(value);
    exit(EXIT_FAILURE);
  }
  return ivalue;
}

template<> QSize SubcommandLineParser::get<QSize>(const QString& name) const
{
  const auto value = get<QString>(name);
  const auto tokens = value.split(RESOLUTION_SEPARATOR);
  static const auto get_int = [](const QString& s, int& i) {
    bool ok;
    i = s.toInt(&ok);
    return ok;
  };
  int w, h;
  if (tokens.size() != 2 || !get_int(tokens[0], w) || !get_int(tokens[1], h)) {
    LERROR << QObject::tr("Expected two integers, separated by '%1' for '%2', but got '%3'.").arg(value);
    exit(EXIT_FAILURE);
  }
  return QSize(w, h);
}

void SubcommandLineParser::print_help() const
{
  std::cout << "Following sub-commands are available:\n";
  for (auto&& [command, cmd_options_list] : options) {
    std::cout << command.toStdString() << "\n";
  }
}



}  // namespace omm
