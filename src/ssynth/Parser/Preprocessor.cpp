#include <ssynth/Logging.h>
#include <ssynth/Parser/Preprocessor.h>

#include <QRandomGenerator>
#include <QRegularExpression>
#include <QStringList>

#include <map>
namespace ssynth
{
using namespace Exceptions;
using namespace Logging;
namespace Parser
{

auto Preprocessor::Process(const QString& input, int seed) -> QString
{
  QRandomGenerator rg(seed);

  QStringList in = input.split(QRegularExpression("\r\n|\r|\n"));

  std::map<QString, QString> substitutions;

  QRegularExpression defineCommand(
      R"(^#define\s([^\s]+)\s(.*)*$)"); // Look for #define varname value
  QRegularExpression defineFloatCommand(
      R"(^#define\s([^\s]+)\s(.*)\s\(float:([^\s]*)\)$)"); // Look for #define varname value
  QRegularExpression defineIntCommand(
      R"(^#define\s([^\s]+)\s(.*)\s\(int:([^\s]*)\)$)"); // Look for #define varname value

  // Match a number: [-+]?[0-9]*\.?[0-9]+
  QRegularExpression randomNumber(
      R"(random\[([-+]?[0-9]*\.?[0-9]+),([-+]?[0-9]*\.?[0-9]+)\])"); // random[-2.3,3.4]

  for (QString& it : in)
  {
    if (it.startsWith("#"))
    {
      // Preprocessor command
      const auto defineMatches = defineCommand.match(it);
      const auto defineFloatMatches = defineFloatCommand.match(it);
      const auto defineIntMatches = defineIntCommand.match(it);

      if (defineFloatMatches.hasMatch())
      {
        //INFO(QString("Found ppC (%1)->(%2): ").arg(defineCommandWithGUI.cap(1)).arg(defineCommandWithGUI.cap(2)) + *it);
        if (defineFloatMatches.captured(2).contains(defineFloatMatches.captured(1)))
        {
          WARNING(QString("#define command is recursive - skipped: %1 -> %2")
                      .arg(defineFloatMatches.captured(1))
                      .arg(defineFloatMatches.captured(2)));
        }
        //substitutions[defineCommandWithGUI.cap(1)] = defineCommandWithGUI.cap(2);
        QString defaultValue = defineFloatMatches.captured(2);
        QString floatInterval = defineFloatMatches.captured(3);
        QStringList fi = floatInterval.split("-");
        if (fi.size() != 2)
        {
          WARNING("Could not understand #define gui command: " + floatInterval);
          continue;
        }
        bool succes = false;
        double d1 = fi[0].toDouble(&succes);
        bool succes2 = false;
        double d2 = fi[1].toDouble(&succes2);
        if (!succes || !succes2)
        {
          WARNING(
              "Could not parse float interval in #define gui command: " + floatInterval);
          continue;
        }
        bool succes3 = false;
        double d3 = defineFloatMatches.captured(2).toDouble(&succes3);
        if (!succes3)
        {
          WARNING(
              "Could not parse default argument in #define gui command: "
              + defineFloatMatches.captured(2));
          continue;
        }
        auto* fp = new FloatParameter(defineFloatMatches.captured(1), d1, d2, d3);
        params.push_back(fp);
      }
      else if (defineIntMatches.hasMatch())
      {
        INFO("INT");
        if (defineIntMatches.captured(2).contains(defineIntMatches.captured(1)))
        {
          WARNING(QString("#define command is recursive - skipped: %1 -> %2")
                      .arg(defineIntMatches.captured(1))
                      .arg(defineIntMatches.captured(2)));
        }
        QString defaultValue = defineIntMatches.captured(2);
        QString intInterval = defineIntMatches.captured(3);
        QStringList ii = intInterval.split("-");
        if (ii.size() != 2)
        {
          WARNING("Could not understand #define gui command: " + intInterval);
          continue;
        }
        bool succes = false;
        int i1 = ii[0].toInt(&succes);
        bool succes2 = false;
        int i2 = ii[1].toInt(&succes2);
        if (!succes || !succes2)
        {
          WARNING("Could not parse int interval in #define gui command: " + intInterval);
          continue;
        }
        bool succes3 = false;
        int i3 = defineIntMatches.captured(2).toInt(&succes3);
        if (!succes3)
        {
          WARNING(
              "Could not parse default argument in #define gui command: "
              + defineIntMatches.captured(2));
          continue;
        }
        auto* fp = new IntParameter(defineIntMatches.captured(1), i1, i2, i3);
        params.push_back(fp);
      }
      else if (defineMatches.hasMatch())
      {
        if (defineMatches.captured(2).contains(defineMatches.captured(1)))
        {
          WARNING(QString("#define command is recursive - skipped: %1 -> %2")
                      .arg(defineMatches.captured(1))
                      .arg(defineMatches.captured(2)));
        }
        substitutions[defineMatches.captured(1)] = defineMatches.captured(2);
      }
      else
      {
        WARNING("Could not understand preprocessor command: " + it);
      }
    }
    else
    {
      // Non-preprocessor command
      // Check for substitutions.
      auto it2 = substitutions.cbegin();
      int subst = 0;
      while (it2 != substitutions.cend())
      {
        if (subst > 100)
        {
          WARNING("More than 100 recursive preprocessor substitutions... breaking.");
          break;
        }
        if (it.contains(it2->first))
        {
          //INFO("Replacing: " + it2.key() + " with " + it2.value());
          it.replace(it2->first, it2->second);

          it2 = substitutions.cbegin();
          subst++;
        }
        else
        {
          it2++;
        }
      }
    }

    auto randNumMatches = randomNumber.globalMatch(it);
    while (randNumMatches.hasNext())
    {
      auto randNumMatch = randNumMatches.next();
      double d1 = randNumMatch.captured(1).toDouble();
      double d2 = randNumMatch.captured(2).toDouble();
      double r = rg.generateDouble() * (d2 - d1) + d1;
      INFO(QString("Random number: %1 -> %2 ").arg(randomNumber.cap(0)).arg(r));
      it.replace(randNumMatch.captured(0), QString::number(r));
    }
  }

  QStringList out = in;
  return out.join("\r\n");
}
}
}
