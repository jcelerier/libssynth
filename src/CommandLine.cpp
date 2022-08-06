#include <ssynth/Logging.h>
#include <ssynth/Model/Builder.h>
#include <ssynth/Model/Rendering/ObjRenderer.h>
#include <ssynth/Model/Rendering/TemplateRenderer.h>
#include <ssynth/Parser/EisenParser.h>
#include <ssynth/Parser/Preprocessor.h>
#include <ssynth/Parser/Tokenizer.h>

#include <QCoreApplication>
#include <QDebug>

class QLogger : public ssynth::Logging::Logger
{
public:
  void log(QString message, ssynth::Logging::LogLevel priority) override
  {
    using namespace ssynth;
    switch (priority)
    {
      default:
      case Logging::LogLevel::NoneLevel:
      case Logging::LogLevel::InfoLevel:
        qInfo() << message;
        break;
      case Logging::LogLevel::DebugLevel:
        qDebug() << message;
        break;
      case Logging::LogLevel::WarningLevel:
        qWarning() << message;
        break;
      case Logging::LogLevel::CriticalLevel:
        qCritical() << message;
        break;
    }
  }
};

auto main(int argc, char** argv) -> int
{
  QCoreApplication app(argc, argv);

  QString input;
  if (argc > 1)
  {
    QFile f(argv[1]);
    f.open(QIODevice::ReadOnly);
    input = f.readAll();
  }
  else if (argc == 1)
  {
    input = R"_(set maxdepth 2000
{ a 0.9 hue 30 } R1

rule R1 w 10 {
{ x 1  rz 3 ry 5  } R1
{ s 1 1 0.1 sat 0.9 } box
}

rule R1 w 10 {
{ x 1  rz -3 ry 5  } R1
{ s 1 1 0.1 } box
}
)_";
  }

  // QLogger l;
  try
  {
    ssynth::Parser::Preprocessor p;
    auto preprocessed = p.Process(input);

    ssynth::Parser::Tokenizer t{std::move(preprocessed)};
    ssynth::Parser::EisenParser e{t};

    auto ruleset = std::unique_ptr<ssynth::Model::RuleSet>{e.parseRuleset()};
    ruleset->resolveNames();
    ruleset->dumpInfo();

    QTextStream ts(stdout);
    if (argc > 2)
    {
      QFile tplFile(argv[2]);
      ssynth::Model::Rendering::Template tpl{tplFile};
      ssynth::Model::Rendering::TemplateRenderer tr{tpl};
      ssynth::Model::Builder b(&tr, ruleset.get(), true);
      b.build();

      ts << tr.getOutput();
    }
    else
    {
      ssynth::Model::Rendering::ObjRenderer obj{10, 10, true, false};
      ssynth::Model::Builder b(&obj, ruleset.get(), true);
      b.build();

      obj.writeToStream(ts);
    }
    ts.flush();
  }
  catch (std::exception& e)
  {
    fprintf(stderr, "%s\n", e.what());
  }
}
