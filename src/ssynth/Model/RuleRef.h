#pragma once

#include <ssynth/Model/Rule.h>

namespace ssynth
{
namespace Model
{

/// A RuleRef holds a pointer to a rule.
/// Its is a placeholder, since rule are parsed as symbolic references,
/// and need to be resolved into actual pointers after the complete parsing of the script.
class RuleRef
{
public:
  RuleRef(QString namedReference)
      : reference(namedReference)
  {
  }
  ~RuleRef() = default;

  Rule* rule() { return rulePtr; }

  QString getReference() const { return reference; }

  void setRef(Rule* rule) { rulePtr = rule; }

private:
  Rule* rulePtr{};
  QString reference;
};

}
}
