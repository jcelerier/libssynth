#pragma once

#include <QString>
#include <QTime>

#include <vector>
#include <stack>

namespace ssynth
{
namespace Logging
{
/// Predefined logging levels
enum LogLevel
{
  NoneLevel,
  DebugLevel,
  TimingLevel,
  InfoLevel,
  WarningLevel,
  CriticalLevel,
  AllLevel
};

/// Abstract base class for all loggers
class Logger
{
public:
  /// The destructors and constructors automatically add to the list of installed loggers.
  Logger()
  { /* loggers.push_back(this); */
  }

  virtual ~Logger()
  {
    /*
    // Remove from list of available loggers.
    for (int i = loggers.size() - 1; i >= 0; i--)
    {
      if (loggers[i] == this)
        loggers.remove(i);
    }
    */
  }

  /// This method all loggers must implement
  virtual void log(QString message, LogLevel priority) = 0;

  /// Log messages are sent to this list of loggers.
  static std::vector<Logger*> loggers;
  static std::stack<QTime> timeStack;
  static std::stack<QString> timeStringStack;

private:
};
/*
void LOG(QString message, LogLevel priority);

/// Useful aliases

void Debug(QString text);
void INFO(QString text);
void TIME(QString text);
void TIME(int repetitions = 0); // End time...
void WARNING(QString text);
void CRITICAL(QString text);
*/

#define LOG(...)
#define Debug(...)
#define INFO(...)
#define TIME(...)
#define WARNING(...)
#define CRITICAL(...)
}
}
