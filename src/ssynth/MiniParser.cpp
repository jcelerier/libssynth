#include <ssynth/Logging.h>
#include <ssynth/MiniParser.h>

namespace ssynth::Misc
{

using namespace Logging;
MiniParser::MiniParser(const QString& value, QChar separator)
    : separator(separator)
    , original(value)
    , value(value)
    , paramCount(0)
{
}

auto MiniParser::getInt(int& val) -> MiniParser&
{
  paramCount++;
  QString first = value.section(separator, 0, 0);
  value = value.section(separator, 1);

  if (first.isEmpty())
  {
    WARNING(QString("Expected argument number %1 for %2").arg(paramCount).arg(original));
  }

  bool succes = false;
  int i = first.toInt(&succes);
  if (!succes)
  {
    WARNING(QString("Expected argument number %1 to be an integer. Found: %2")
                .arg(paramCount)
                .arg(first));
  }
  val = i;

  return *this;
}

auto MiniParser::getBool(bool& val) -> MiniParser&
{
  paramCount++;
  QString first = value.section(separator, 0, 0);
  value = value.section(separator, 1);

  if (first.isEmpty())
  {
    WARNING(QString("Expected argument number %1 for %2").arg(paramCount).arg(original));
  }

  if (first.toLower() == QStringLiteral("true"))
  {
    val = true;
  }
  else if (first.toLower() == QStringLiteral("false"))
  {
    val = false;
  }
  else
  {
    WARNING(QString("Expected argument number %1 to be either true or false. Found: %2")
                .arg(paramCount)
                .arg(first));
  }

  return *this;
}

auto MiniParser::getDouble(double& val) -> MiniParser&
{
  paramCount++;
  QString first = value.section(separator, 0, 0);
  value = value.section(separator, 1);

  if (first.isEmpty())
  {
    WARNING(QString("Expected argument number %1 for %2").arg(paramCount).arg(original));
  }

  bool succes = false;
  double d = first.toDouble(&succes);
  if (!succes)
  {
    WARNING(QString("Expected argument number %1 to be an double. Found: %2")
                .arg(paramCount)
                .arg(first));
  }
  val = d;

  return *this;
}

auto MiniParser::getFloat(float& val) -> MiniParser&
{
  paramCount++;
  QString first = value.section(separator, 0, 0);
  value = value.section(separator, 1);

  if (first.isEmpty())
  {
    WARNING(QString("Expected argument number %1 for %2").arg(paramCount).arg(original));
  }

  bool succes = false;
  float d = first.toFloat(&succes);
  if (!succes)
  {
    WARNING(QString("Expected argument number %1 to be an float. Found: %2")
                .arg(paramCount)
                .arg(first));
  }
  val = d;

  return *this;
}

}
