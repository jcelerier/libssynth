#include <ssynth/Parser/Tokenizer.h>

#include <QRegularExpression>
#include <QStringList>
namespace ssynth::Parser
{

Tokenizer::~Tokenizer() = default;

Tokenizer::Tokenizer(QString input)
    : currentSymbol(-1)
{

  QStringList operators;
  operators << "c"
            << "reflect"
            << "color"
            << "blend"
            << "a"
            << "alpha"
            << "matrix"
            << "h"
            << "hue"
            << "sat"
            << "b"
            << "brightness"
            << "v"
            << "x"
            << "y"
            << "z"
            << "rx"
            << "ry"
            << "rz"
            << "s"
            << "fx"
            << "fy"
            << "fz"
            << "maxdepth"
            << "weight"
            << "md"
            << "w";

  // We will split on whitespace and line breaks
  // TODO: Respect quotations ( file = "C:\Program Files\Test" )
  QStringList l;
  std::vector<int> positions;

  // We will use our own split routine
  QString current;
  int startPos = 0;
  input += " "; // to ensure last symbol gets parsed.
  bool inMultiComment = false;
  bool inComment = false;
  int newlines = 0;
  for (int i = 0; i < input.length(); i++)
  {

    if (input.at(i) == '\r')
    {
      inComment = false;
      newlines++;
    }

    // Check if we found a preprocessor comment (there must occur at the beginning of a line.
    if (input.at(i) == '#'
        && ((i == 0) || (input.at(i - 1) == '\r') || (input.at(i - 1) == '\n')))
    {
      inComment = true;
      i++;
      continue;
    }

    if (i < input.length() - 1)
    {
      if (input.at(i) == '*' && input.at(i + 1) == '/')
      {
        inMultiComment = false;
        i++;
        continue;
      }

      if (input.at(i) == '/' && input.at(i + 1) == '/')
      {
        inComment = true;
        i++;
        continue;
      }

      if (input.at(i) == '/' && input.at(i + 1) == '*')
      {
        inMultiComment = true;
        i++;
        continue;
      }
    }

    if (inMultiComment || inComment)
      continue;

    if (input.at(i) == '[')
    {
      while (input.at(i) != ']' && i < input.length())
      {
        current += input.at(i);
        i++;
      }
      current += ']';

      if (input.at(i) != ']')
      {
        throw ParseError("No matching ']' found for '['", startPos);
      }

      l.push_back(current);
      positions.push_back(startPos - newlines);
      startPos = i;
      current = "";
    }
    else if (
        input.at(i) == '{' || input.at(i) == '}' || input.at(i) == ' '
        || (input.at(i) == '\r') || (input.at(i) == '\n'))
    {
      QString trimmed = current.remove(QRegularExpression(R"(\s|\r|\n)"));
      if (!current.trimmed().isEmpty())
      {
        l.push_back(trimmed);
        positions.push_back(startPos - newlines);
      }
      if (input.at(i) == '{' || input.at(i) == '}')
      {
        l.push_back(QString(input.at(i)));
        positions.push_back(i - newlines);
      }
      current = "";
      startPos = i;
    }
    else
    {
      current += input.at(i);
    }
  }

  for (int i = 0; i < l.size(); i++)
  {
    QString s = l[i];
    int pos = positions[i];
    QString sl = l[i].toLower();

    if (sl == "rule")
    {
      symbols.emplace_back(pos, Symbol::Rule, s);
    }
    else if (sl == "{")
    {
      symbols.emplace_back(pos, Symbol::LeftBracket, s);
    }
    else if (sl == ">")
    {
      symbols.emplace_back(pos, Symbol::MoreThan, s);
    }
    else if (sl == "}")
    {
      symbols.emplace_back(pos, Symbol::RightBracket, s);
    }
    else if (sl == "*")
    {
      symbols.emplace_back(pos, Symbol::Multiply, s);
    }
    else if (sl == "set")
    {
      symbols.emplace_back(pos, Symbol::Set, s);
    }
    else if (QString("+-0123456789").contains(s[0]))
    {
      // It is a number (hopefully)

      if (s.count("/") == 1)
      {
        QString s1 = s.section("/", 0, 0);
        QString s2 = s.section("/", 1, 1);
        bool succes1 = false;
        int i1 = s1.toInt(&succes1);
        bool succes2 = false;
        int i2 = s2.toInt(&succes2);

        if ((i1 && i2) && (i2 != 0))
        {
          Symbol ns(pos, Symbol::Number, s);
          ns.isInteger = false;
          ns.floatValue = ((double)i1) / i2;
          symbols.push_back(ns);
          continue;
        }
        else
        {
          throw ParseError("Invalid fraction found: " + s, pos);
        }
      }

      bool succes = false;
      int i = s.toInt(&succes);

      if (succes)
      {
        Symbol ns(pos, Symbol::Number, s);
        ns.isInteger = true;
        ns.intValue = i;
        symbols.push_back(ns);
        continue;
      }

      // the number was not an integer... Is it a floating-point value?

      double d = s.toDouble(&succes);

      if (succes)
      {
        Symbol ns(pos, Symbol::Number, s);
        ns.isInteger = false;
        ns.floatValue = d;
        //INFO(QString("Added float value:%1").arg(ns.floatValue));
        symbols.push_back(ns);
        continue;
      }

      throw ParseError("Invalid symbol found: " + s, pos);
    }
    else if (operators.contains(sl))
    {
      QString longName = sl;

      // Resolve abbreviations
      if (longName == "md")
        longName = "maxdepth";
      if (longName == "w")
        longName = "weight";
      if (longName == "h")
        longName = "hue";
      if (longName == "b")
        longName = "brightness";
      if (longName == "a")
        longName = "alpha";
      if (longName == "c")
        longName = "color";

      Symbol ns(pos, Symbol::Operator, longName);
      symbols.push_back(ns);
    }
    else
    {
      // TODO: We should check syntax of userstring here... (we dont want strings like slk"{/})
      Symbol ns(pos, Symbol::UserString, sl);
      symbols.push_back(ns);
    }
  }

  for (int i = 0; i < symbols.size(); i++)
  {

    //INFO(QString("%1. Symbol: %2, Position: %3").arg(i).arg(symbols[i].text).arg(positions[i]));
  }
}

auto Tokenizer::getSymbol() -> Symbol
{
  currentSymbol++;

  if (currentSymbol < symbols.size())
  {
    return symbols[currentSymbol];
  }

  return {-1, Symbol::End, "#END#"};
}
}
