#include "subcommandlineparser.h"
#include "logging.h"
#include <QSize>
#include <iostream>

QCommandLineOption make_verbosity_option()
{
  return {{"v", "verbosity"},
          QObject::tr("The verbosity (fatal|critical|warning|info|debug)."),
          QObject::tr("LEVEL")};
}

QCommandLineOption make_input_option()
{
  return {{"f", "input"}, QObject::tr("The input scene file."), QObject::tr("FILENAME")};
}

static const std::map<QString, QList<QCommandLineOption>> options{
    {omm::SubcommandLineParser::COMMAND_RENDER,
     {make_input_option(),
      make_verbosity_option(),
      {{"o", "output"},
       QObject::tr("Where to save the batch renderings. Use `%1` as framenumber placeholder.")
           .arg(QString(omm::SubcommandLineParser::FRAMENUMBER_PLACEHOLDER).repeated(4)),
       QObject::tr("FILENAME")},
      {{"y", "overwrite"}, QObject::tr("Overwrite existing files without warning.")},
      {{"w", "width"},
       QObject::tr("The width of the rendering. "
                   "Height is calculated based on the aspect ratio of the selected view."),
       QObject::tr("W"),
       QString("1000")},
      {{"s", "start-frame"},
       QObject::tr("start-frame of the rendering (optional)."),
       QObject::tr("#FRAME"),
       "1"},
      {
          {"V", "view"},
          QObject::tr("name of the view."),
          QObject::tr("NAME"),
      },
      {
          {"q", "object"},
          QObject::tr("The name of the object to render. "
                      "Keep clear to render the whole scene. "
                      "The name does not need to be unique. "
                      "In this case, all matching objects are rendered. "
                      "Perl-like regular expressions are supported."),
          QObject::tr("NAME"),
          "",
      },
      {
          {"u", "unique"},
          QObject::tr("If this option is set, the target object (see 'path' or 'object' argument) "
                      "must be unique, i.e., there must be exactly one such object. "
                      "The application terminates if this requirement is violated."),
      },
      {
          {"p", "path"},
          QObject::tr("The path of the object to render. "
                      "Keep clear to render the whole scene. "
                      "The path does not need to be unique. "
                      "In this case, all matching objects are rendered. "
                      "Perl-like regular expressions are supported."),
          QObject::tr("NAME"),
          "",
      },
      {{"n", "sequence-length"},
       QObject::tr("number of frames (optional)."),
       QObject::tr("#FRAMES"),
       "1"},
      {
          {"G", "no-opengl"},
          QObject::tr("disable OpenGL. OpenGL is enabled by default when using the `%1'-command.")
              .arg(omm::SubcommandLineParser::COMMAND_RENDER),
      }}},
    {omm::SubcommandLineParser::COMMAND_TREE,
     {
         make_input_option(),
         make_verbosity_option(),
     }},
    {omm::SubcommandLineParser::STATUS_CODE,
     {make_verbosity_option(),
      {
          {"c", "get-code"},
          QObject::tr("Get status code for given description."),
          QObject::tr("DESCRIPTION"),
      },
      {{"l", "list"}, QObject::tr("List status codes and descriptions.")}}}};

namespace omm
{
SubcommandLineParser::SubcommandLineParser(int argc, char* argv[])
    : m_appname(argv[0]), m_command(argc == 1 ? "" : argv[1])
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

SubcommandLineParser::SubcommandLineParser()
= default;

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
    LERROR << QObject::tr("Expected integer argument for '%1', but got '%2'.").arg(name, value);
    exit(EXIT_FAILURE);
  }
  return ivalue;
}

void SubcommandLineParser::print_help() const
{
  std::cout << "Following sub-commands are available:\n";
  for (auto&& [command, cmd_options_list] : options) {
    std::cout << command.toStdString() << "\n";
  }
}

}  // namespace omm
