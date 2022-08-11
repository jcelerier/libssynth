#include <ssynth/Logging.h>
#include <ssynth/Parser/Preprocessor.h>

#include <QRandomGenerator>
#include <QStringList>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QRegExp>
#endif


#include <map>
namespace ssynth
{
using namespace Exceptions;
using namespace Logging;
namespace Parser
{

auto Preprocessor::Process(const QString& input, int seed) -> QString
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  QRandomGenerator rg(seed);

  QStringList in = input.split(QRegExp("\r\n|\r|\n"));

  std::map<QString, QString> substitutions;

  QRegExp ppCommand("^#"); // Look for #define varname value
  QRegExp defineCommand(
      R"(^#define\s([^\s]+)\s(.*)*$)"); // Look for #define varname value
  QRegExp defineFloatCommand(
      R"(^#define\s([^\s]+)\s(.*)\s\(float:([^\s]*)\)$)"); // Look for #define varname value
  QRegExp defineIntCommand(
      R"(^#define\s([^\s]+)\s(.*)\s\(int:([^\s]*)\)$)"); // Look for #define varname value

  // Match a number: [-+]?[0-9]*\.?[0-9]+
  QRegExp randomNumber(
      R"(random\[([-+]?[0-9]*\.?[0-9]+),([-+]?[0-9]*\.?[0-9]+)\])"); // random[-2.3,3.4]

  for (auto& it : in)
  {

    if (ppCommand.indexIn(it) != -1)
    {
      // Preprocessor command

      if (defineFloatCommand.indexIn(it) != -1)
      {
        //INFO(QString("Found ppC (%1)->(%2): ").arg(defineCommandWithGUI.cap(1)).arg(defineCommandWithGUI.cap(2)) + *it);
        if (defineFloatCommand.cap(2).contains(defineFloatCommand.cap(1)))
        {
          WARNING(QString("#define command is recursive - skipped: %1 -> %2")
                      .arg(defineFloatCommand.cap(1))
                      .arg(defineFloatCommand.cap(2)));
        }
        //substitutions[defineCommandWithGUI.cap(1)] = defineCommandWithGUI.cap(2);
        QString defaultValue = defineFloatCommand.cap(2);
        QString floatInterval = defineFloatCommand.cap(3);
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
        double d3 = defineFloatCommand.cap(2).toDouble(&succes3);
        if (!succes3)
        {
          WARNING(
              "Could not parse default argument in #define gui command: "
              + defineFloatCommand.cap(2));
          continue;
        }
        auto* fp = new FloatParameter(defineFloatCommand.cap(1), d1, d2, d3);
        params.push_back(fp);
      }
      else if (defineIntCommand.indexIn(it) != -1)
      {
        INFO("INT");
        if (defineIntCommand.cap(2).contains(defineIntCommand.cap(1)))
        {
          WARNING(QString("#define command is recursive - skipped: %1 -> %2")
                      .arg(defineIntCommand.cap(1))
                      .arg(defineIntCommand.cap(2)));
        }
        QString defaultValue = defineIntCommand.cap(2);
        QString intInterval = defineIntCommand.cap(3);
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
        int i3 = defineIntCommand.cap(2).toInt(&succes3);
        if (!succes3)
        {
          WARNING(
              "Could not parse default argument in #define gui command: "
              + defineIntCommand.cap(2));
          continue;
        }
        auto* fp = new IntParameter(defineIntCommand.cap(1), i1, i2, i3);
        params.push_back(fp);
      }
      else if (defineCommand.indexIn(it) != -1)
      {
        if (defineCommand.cap(2).contains(defineCommand.cap(1)))
        {
          WARNING(QString("#define command is recursive - skipped: %1 -> %2")
                      .arg(defineCommand.cap(1))
                      .arg(defineCommand.cap(2)));
        }
        substitutions[defineCommand.cap(1)] = defineCommand.cap(2);
      }
      else
      {
        WARNING("Could not understand preprocessor command: " + *it);
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

    while (randomNumber.indexIn(it) != -1)
    {
      double d1 = randomNumber.cap(1).toDouble();
      double d2 = randomNumber.cap(2).toDouble();
      double r = rg.generateDouble() * (d2 - d1) + d1;
      INFO(QString("Random number: %1 -> %2 ").arg(randomNumber.cap(0)).arg(r));
      it.replace(randomNumber.cap(0), QString::number(r));
    }
  }

  QStringList out = in;
  return out.join("\r\n");
#else
  return {};
#endif
}
}
}
