#pragma once

#include <ssynth/Exception.h>

#include <QString>

#include <vector>

namespace ssynth
{
namespace Parser
{

class GuiParameter
{
public:
  GuiParameter(QString name)
      : name(name){};
  virtual ~GuiParameter() = default;
  virtual QString getName() { return name; }

protected:
  QString name;
};

class FloatParameter final : public GuiParameter
{
public:
  FloatParameter(QString name, double from, double to, double defaultValue)
      : GuiParameter(name)
      , from(from)
      , to(to)
      , defaultValue(defaultValue){};

  double getFrom() { return from; }
  double getTo() { return to; }
  double getDefaultValue() { return defaultValue; }

private:
  double from;
  double to;
  double defaultValue;
};

class IntParameter final : public GuiParameter
{
public:
  IntParameter(QString name, int from, int to, int defaultValue)
      : GuiParameter(name)
      , from(from)
      , to(to)
      , defaultValue(defaultValue){};

  int getFrom() { return from; }
  int getTo() { return to; }
  int getDefaultValue() { return defaultValue; }

private:
  int from;
  int to;
  int defaultValue;
};

/// The preprocessor is responsible for expanding '#define'
///
class Preprocessor
{

public:
  Preprocessor() = default;

  // The preprocess replaces 'random[2,4]' statements with random numbers.
  // This requires a seed. Using the same seed as controls the EisenScript is probably the best idea her.
  QString Process(const QString& input, int seed = 0);
  std::vector<GuiParameter*> getParameters() { return params; }

private:
  std::vector<GuiParameter*> params;
};

}
}
